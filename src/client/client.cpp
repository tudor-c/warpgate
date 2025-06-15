#include <format>

#include "client.h"
#include "consts.h"
#include "log.h"
#include "../Task.h"

Client::Client(
    const std::string &trackerHost,
    const int trackerPort,
    [[maybe_unused]] bool registerAsWorker, // TODO use
    const std::string &taskConfigPath,
    const std::string& taskLibPath) :
        mTrackerHost(trackerHost),
        mTrackerPort(trackerPort),
        mTrackerConnection(trackerHost, trackerPort),
        mOwnServer(FREE_PORT)
        {

    lg::info("Starting worker at {}:{} 🌐",
        LOCALHOST, mOwnServer.port());

    this->bindRcpServerFunctions();

    if (!taskConfigPath.empty() && !taskLibPath.empty()) {
        std::unique_ptr<Task> task;
        try {
            task = std::make_unique<Task>(taskConfigPath);
        }
        catch (std::runtime_error& e) {
            spdlog::error(e.what());
        }

        mOwnLibContent = util::readBinaryFile(taskLibPath);
        if (task && !mOwnLibContent.empty()) {
            submitTaskToTracker(*task);
        }
        else {
            throw std::runtime_error(std::format("Could not read task config or lib files!"));
        }
    }
    else if (!taskConfigPath.empty() || !taskLibPath.empty()) {
        throw std::runtime_error(std::format(
            "Both `{}` and `{}` are needed to work together!",
            FLAG_CLIENT_TASK_CONFIG_PATH, FLAG_CLIENT_TASK_LIB_PATH));
    }
}

auto Client::run() -> int {
    if (!registerAsClient()) {
        return 1;
    }

    mServerThread = std::thread(&rpc::server::run, &mOwnServer);
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
}

auto Client::registerAsClient() -> bool {
    mTrackerConnection.set_timeout(TIMEOUT_MS);
    try {
        mOwnId = mTrackerConnection.call(RPC_REGISTER_CLIENT, LOCALHOST, getOwnPort()).as<Id>();
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

auto Client::submitTaskToTracker(const Task &task) -> void {
    mTrackerConnection.call(RPC_SUBMIT_TASK_TO_TRACKER, task);
}

auto Client::receiveJob(const Subtask& subtask) -> bool  {
    if (isBusy()) {
        lg::debug("Denied subtask: id={}, fn={}!", subtask.id, subtask.functionName);
        return false;
    }
    lg::debug("Accepted subtask: id={}, fn={}.", subtask.id, subtask.functionName);
    mJobQueue.push(subtask);
    return true;
}

auto Client::processJobsQueues() -> void {
    this->launchJobsFromQueue();
    this->sendFinishedJobsNotifications();
}

auto Client::launchJobsFromQueue() -> void {
    while (!mJobQueue.empty()) {
        const auto subtask = std::move(mJobQueue.front());
        mJobQueue.pop();

        mWorkerThreads.insert({subtask.id, std::thread([this, subtask]() {
            lg::debug("Starting working on subtask {}...", subtask.functionName);
            const auto previousResults = this->fetchSubtaskParameterData(subtask);
            lg::debug("Previous results:");

            const Id taskId = subtask.taskId;
            if (!mOtherLibsContents.contains(taskId)) {
                mOtherLibsContents[taskId] = std::move(this->fetchTaskLibContent(taskId));
                lg::debug("Copied lib for {}, size is: {}", subtask.functionName,
                    mOtherLibsContents.at(taskId).size());
            }
            for (const auto& res : previousResults) {
                lg::debug(" - {}", res);
            }

            // job done 🧌
            mFinishedJobs.push(subtask.id);
            mResults.insert({subtask.id, std::format("result-of:{}", subtask.functionName)});
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
    return subtask.dependencyIds
        | std::views::transform(
            [this](const Id& dependencyId) {
                return fetchSubtaskResultsFromPeer(dependencyId);
        })
        | std::ranges::to<std::vector>();
}

auto Client::fetchTaskLibContent(const Id taskId) -> std::vector<std::byte> {
    const auto [host, port] = mTrackerConnection.call(RPC_FETCH_TASK_ACQUIRER_ADDRESS,
        taskId).as<SocketAddress>();
    rpc::client acquirer(host, port);
    return acquirer.call(RPC_FETCH_LIB_CONTENT).as<std::vector<std::byte>>();
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
    const auto result = std::move(mResults.at(subtaskId));
    mResults.erase(subtaskId);
    return result;
    // return "'some-data'";
}

auto Client::getOwnPort() const -> int {
    return mOwnServer.port();
}

auto Client::isBusy() const -> bool {
    return mJobQueue.size() + mWorkerThreads.size() >= ACTIVE_JOB_LIMIT;
}
