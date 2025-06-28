#include <format>
#include <fstream>
#include <utility>

#include "client.h"

#include "consts.h"
#include "log.h"
#include "utils.h"
#include "Task.h"

Client::Client(
    const std::string &trackerHost,
    const int trackerPort,
    const bool notWorker,
    std::string  taskConfigPath,
    std::string  outputPath) :
        mTrackerHost(trackerHost),
        mTrackerPort(trackerPort),
        mNotWorker(notWorker),
        mTaskConfigPath(std::move(taskConfigPath)),
        mOutputPath(std::move(outputPath)),
        mTrackerConnection(trackerHost, trackerPort),
        mOwnServer(FREE_PORT) {

    lg::info("Starting worker at {}:{} 🌐",
        LOCALHOST, mOwnServer.port());
    this->bindRcpServerFunctions();
}

auto Client::run() -> int {
    if (!registerAsClient()) {
        return 1;
    }

    if (!readAndSubmitTask()) {
        return 1;
    }

    mServerThread = std::thread([this] {
        this->mOwnServer.run();
    });
    mHeartbeatThread = std::thread([this] {
        util::scheduleTask(
            CLIENT_HEARTBEAT_PERIOD_MS,
            [this] { mTrackerConnection.call(RPC_HEARTBEAT, mOwnId); },
            [this] { return false; }
        );
    });
    mJobThread = std::thread([this] {
        util::scheduleTask(
            CLIENT_JOB_LAUNCH_INTERVAL_MS,
            [this] { this->processJobsQueues(); },
            [this] { return false; }
        );
    });

    teardown();
    return 0;
}

auto Client::bindRcpServerFunctions() -> void {
    mOwnServer.bind(RPC_DISPATCH_JOB, [this](const Subtask& subtask) {
        return this->receiveJob(subtask);
    });
    mOwnServer.bind(RPC_FETCH_SUBTASK_RESULT, [this](const Id subtaskId) {
        return this->extractFinishedJobResult(subtaskId);
    });
    mOwnServer.bind(RPC_FETCH_LIB_CONTENT, [this] {
        return mOwnLibContent;
    });
    mOwnServer.bind(RPC_ANNOUNCE_TASK_COMPLETED, [this](const Id rootId, const SocketAddress& completer) {
        this->processFinishedTask(rootId, completer);
    });
    mOwnServer.bind(RPC_FETCH_SUBTASK_INPUT_DATA, [this](const int subtaskIndex) {
        return mInputData.at(subtaskIndex);
    });
}

auto Client::registerAsClient() -> bool {
    mTrackerConnection.set_timeout(TIMEOUT_MS);
    try {
        mOwnId = mTrackerConnection.call(RPC_REGISTER_CLIENT, LOCALHOST, getOwnPort(), !mNotWorker).as<Id>();
    } catch (std::exception&) {
        lg::error("Tracker unavailable at {}:{}!",
            mTrackerHost, mTrackerPort);
        return false;
    }
    mTrackerConnection.clear_timeout();

    lg::info("Registered as worker for tracker at {}:{}, own ID is {}.",
        mTrackerHost, mTrackerPort, mOwnId);

    return true;
}

auto Client::teardown() -> void {
    mServerThread.join();
    unregisterAsClient();
}

auto Client::unregisterAsClient() -> void {
    lg::info("Unregistered as worker for tracker at {}:{}.",
        mTrackerHost, mTrackerPort);
    mTrackerConnection.call(RPC_UNREGISTER_CLIENT, mOwnId);
}

auto Client::readAndSubmitTask() -> bool {
    if (!mTaskConfigPath.empty()) {
        if (mOutputPath.empty()) {
            mOutputPath = TASK_OUTPUT_DEFAULT_PATH;
        }
        try {
            mOwnTask = std::make_unique<Task>(mTaskConfigPath);
        }
        catch (std::runtime_error& e) {
            lg::error(e.what());
            std::exit(1);
        }

        this->readInputDataFromFiles();
        mTaskLibPath = mOwnTask.get()->getLibPath();
        mOwnLibContent = util::readBinaryFile(mTaskLibPath);
        if (mOwnTask && !mOwnLibContent.empty()) {
            submitTaskToTracker(*mOwnTask);
        }
        else {
            lg::error("Could not read task config or lib files!");
            return false;
        }
    }
    return true;
}

auto Client::submitTaskToTracker(const Task &task) -> void {
    lg::info("Submitting task {} to tracker..", task.getName());
    mTrackerConnection.call(RPC_SUBMIT_TASK_TO_TRACKER, task, mOwnId);
}

auto Client::receiveJob(const Subtask& subtask) -> bool  {
    if (isBusy()) {
        lg::error("Denied subtask: id={}, fn={}!", subtask.id, subtask.functionName);
        return false;
    }
    lg::info("Accepted subtask: id={}, fn={}.", subtask.id, subtask.functionName);
    mJobQueue.push(subtask);
    return true;
}

auto Client::processJobsQueues() -> void {
    this->launchJobsFromQueue();
    this->sendFinishedJobsNotifications();
    this->getFinishedTaskResults();
}

auto Client::launchJobsFromQueue() -> void {
    while (!mJobQueue.empty()) {
        const auto subtask = std::move(mJobQueue.front());
        mJobQueue.pop();

        mWorkerThreads.insert({subtask.id, std::thread([this, subtask]() {
            const auto previousResults = this->fetchSubtaskParameterData(subtask);

            if (!mOtherTaskLibs.contains(subtask.taskId)) {
                this->fetchAndLoadTaskLibContent(subtask);
            }
            const auto func = mOtherTaskLibs.at(subtask.taskId).loadFunction(
                subtask.functionName);
            auto result = func(previousResults);
            lg::info("Finished subtask {}!", subtask.functionName);

            // job done 🧌
            mFinishedJobs.push(subtask.id);
            mResults.insert({subtask.id, std::format("result_of_{}: {}", subtask.functionName,
                result)});
        })});
    }
}

auto Client::fetchSubtaskResultsFromPeer(const Id subtaskId) -> ResultType {
    const auto [host, port] = mTrackerConnection.call(RPC_FETCH_JOB_COMPLETER_ADDRESS,
        subtaskId).as<SocketAddress>();
    rpc::client peerConnection(host, port);
    return peerConnection.call(RPC_FETCH_SUBTASK_RESULT, subtaskId)
        .as<ResultType>();
}

auto Client::fetchSubtaskParameterData(const Subtask& subtask) -> std::vector<ResultType> {
    if (subtask.dependencyIds.size() > 0) {
        return subtask.dependencyIds
            | std::views::transform(
                [this](const Id& dependencyId) {
                    return fetchSubtaskResultsFromPeer(dependencyId);
            })
            | std::ranges::to<std::vector>();
    }
    if (subtask.inputDataPath.empty()) {
        return {};
    }
    return {fetchSubtaskInputData(subtask)};
}

auto Client::fetchSubtaskInputData(const Subtask& subtask) -> ResultType {
    const auto [host, port] = mTrackerConnection.call(RPC_FETCH_SUBTASK_ACQUIRER_ADDRESS, subtask.id)
        .as<SocketAddress>();
    rpc::client peerConnection(host, port);
    return peerConnection.call(RPC_FETCH_SUBTASK_INPUT_DATA, subtask.index).as<ResultType>();
}

auto Client::fetchTaskLibContent(const Id taskId) -> std::vector<unsigned char> {
    lg::debug("Fetching lib for taskId {}", taskId);
    const auto [host, port] = mTrackerConnection.call(RPC_FETCH_TASK_ACQUIRER_ADDRESS,
        taskId).as<SocketAddress>();
    rpc::client acquirer(host, port);
    return acquirer.call(RPC_FETCH_LIB_CONTENT).as<std::vector<unsigned char>>();
}

auto Client::sendFinishedJobsNotifications() -> void {
    while (!mFinishedJobs.empty()) {
        const auto jobId = mFinishedJobs.front();
        mFinishedJobs.pop();

        mWorkerThreads.at(jobId).join();
        mWorkerThreads.erase(jobId);
        lg::debug("Announcing finished job: {}...", jobId);
        mTrackerConnection.call(RPC_ANNOUNCE_SUBTASK_COMPLETED, mOwnId, jobId);
    }
}

auto Client::extractFinishedJobResult(const Id subtaskId) -> ResultType {
    lg::debug("Fetching result for subtask {}..", subtaskId);
    const auto result = std::move(mResults.at(subtaskId));
    mResults.erase(subtaskId);
    return result;
}

auto Client::fetchAndLoadTaskLibContent(const Subtask& subtask) -> void {
    const auto& taskId = subtask.taskId;
    const auto& libContent = this->fetchTaskLibContent(taskId);
    char tempTemplate[] = "/tmp/tempXXXXXX";

    const int fd = mkstemp(tempTemplate);
    if (fd == -1) {
        throw std::runtime_error("Error creating temporary file\n");
    }
    close(fd);
    std::ofstream dynLibFile(tempTemplate, std::ios::binary);

    dynLibFile.write(reinterpret_cast<const char*>(libContent.data()), static_cast<long>(libContent.size()));
    dynLibFile.close();

    mOtherTaskLibs.insert({taskId, DynamicLibrary(tempTemplate)});
    lg::debug("Loaded lib for {}, size is: {}B", subtask.functionName, libContent.size());

    unlink(tempTemplate);
}

auto Client::processFinishedTask(Id rootId, const SocketAddress &taskCompleter) -> void {
    mTaskCompleter = std::make_optional(std::make_pair(rootId, taskCompleter));
}

auto Client::getFinishedTaskResults() -> void {
    if (!mTaskCompleter.has_value()) {
        return;
    }
    const auto& [rootId, addr] = mTaskCompleter.value();
    lg::info("Own task {} is completed, fetching results from {}..",
        mOwnTask->getName(), addr.toString());

    rpc::client peerConnection(addr.host, addr.port);
    const auto result = peerConnection.call(RPC_FETCH_SUBTASK_RESULT, rootId).as<ResultType>();
    lg::info("Received task results!");
    mTaskCompleter = std::nullopt;
    this->writeOutputToFile(result);
}

auto Client::writeOutputToFile(const ResultType& data) const -> void {
    std::ofstream outFile;
    outFile.open(mOutputPath);
    if (!outFile.is_open()) {
        throw std::runtime_error(std::format(
            "Could not create output file at {}", mOutputPath));
    }
    outFile << data;
    lg::debug("Saved output to {}", mOutputPath);
}

auto Client::readInputDataFromFiles() -> void {
    for (const auto& subtask : mOwnTask->getSubtasks() | std::views::values) {
        if (subtask.inputDataPath.empty()) {
            continue;
        }
        mInputData.insert({subtask.index, util::readTextFile(subtask.inputDataPath)});
    }
}

auto Client::getOwnPort() const -> int {
    return mOwnServer.port();
}

auto Client::isBusy() const -> bool {
    return mJobQueue.size() + mWorkerThreads.size() >= ACTIVE_JOB_LIMIT;
}
