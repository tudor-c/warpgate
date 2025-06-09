#include <format>

#include "client.h"
#include "consts.h"
#include "log.h"
#include "../Task.h"

Client::Client(
    const std::string &trackerHost,
    const int trackerPort,
    [[maybe_unused]] bool registerAsWorker, // TODO use
    const std::string &taskConfigPath) :
        mTrackerHost(trackerHost),
        mTrackerPort(trackerPort),
        mTrackerConn(trackerHost, trackerPort),
        mOwnServer(FREE_PORT),
        mExecutorPool(std::vector<std::future<std::string>>()){

    lg::info("Starting worker at {}:{} 🌐",
        LOCALHOST, mOwnServer.port());

    this->bindRcpServerFunctions();

    if (!taskConfigPath.empty()) {
        std::unique_ptr<Task> task;
        try {
            task = std::make_unique<Task>(taskConfigPath);
        }
        catch (std::runtime_error& e) {
            spdlog::error(e.what());
        }

        if (task) {
            submitTaskToTracker(*task);
        }
    }
}

Client::~Client() {}

auto Client::run() -> int {
    if (!registerAsClient()) {
        return 1;
    }

    mServerThread = std::thread(&rpc::server::run, &mOwnServer);
    mHeartbeatThread = std::thread([this] {
        util::scheduleTask(
            CLIENT_HEARTBEAT_PERIOD_MS,
            [this] { mTrackerConn.call(RPC_HEARTBEAT, mOwnId); },
            [this] { return false; }
        );
    });
    mJobThread = std::thread([this] {
        util::scheduleTask(
            CLIENT_JOB_LAUNCH_INTERVAL_MS,
            [this] { this->executeJobsFromQueue(); },
            [this] { return false; }
        );
    });

    mTrackerConn.call(RPC_TEST_ANNOUNCEMENT,
        std::format("Hello world! I'm {}:{}", LOCALHOST, mOwnServer.port()));

    teardown();
    return 0;
}

auto Client::bindRcpServerFunctions() -> void {
    mOwnServer.bind(RPC_TEST_ANNOUNCEMENT_BROADCAST, [](const std::string& mess) {
        lg::debug("Announcement from another peer: {}", mess);
    });
    mOwnServer.bind(RPC_DISPATCH_JOB, [this](const Subtask& subtask) {
        return this->receiveJob(subtask);
    });
}

auto Client::registerAsClient() -> bool {
    mTrackerConn.set_timeout(TIMEOUT_MS);
    try {
        mOwnId = mTrackerConn.call(RPC_REGISTER_CLIENT, LOCALHOST, getOwnPort()).as<Id>();
    } catch (std::exception& e) {
        lg::error("Could not connect to tracker at {}:{}!\n {}",
            mTrackerHost, mTrackerPort, e.what());
        return false;
    }
    mTrackerConn.clear_timeout();

    lg::info("Registered as worker for tracker at {}:{}, own ID is {}",
        mTrackerHost, mTrackerPort, mOwnId);

    return true;
}

auto Client::teardown() -> void {
    mServerThread.join();
    unregisterAsClient();
}

auto Client::unregisterAsClient() -> void {
    lg::info("Unregistered as worker for tracker at {}:{}",
        mTrackerHost, mTrackerPort);
    mTrackerConn.call(RPC_UNREGISTER_CLIENT, mOwnId);
}

auto Client::submitTaskToTracker(const Task &task) -> void {
    mTrackerConn.call(RPC_SUBMIT_TASK_TO_TRACKER, task);
}

auto Client::receiveJob(const Subtask& subtask) -> bool  {
    if (isBusy()) {
        lg::debug("Denied subtask: id={}, fn={}", subtask.id, subtask.functionName);
        return false;
    }
    lg::debug("Accepted subtask: id={}, fn={}", subtask.id, subtask.functionName);
    mJobQueue.push(subtask);
    return true;
}

auto Client::executeJobsFromQueue() -> void {
    if (mJobQueue.empty()) {
        return;
    }

    const auto subtask = std::move(mJobQueue.front());
    mJobQueue.pop();
    lg::info("Executing subtask {}", subtask.functionName);

    mTrackerConn.call(RPC_ANNOUNCE_SUBTASK_COMPLETED, mOwnId, subtask.id);

    // for (auto it = mExecutorPool.begin(); it != mExecutorPool.end();) {
    //     auto status = it->wait_for(std::chrono::milliseconds(0));
    //     if (status == std::future_status::ready) {
    //         lg::debug("Done: {}", it->get());
    //         mExecutorPool.erase(it++);
    //     } else {
    //         ++it;
    //     }
    //
    // }
    // while (!mJobQueue.empty() && mExecutorPool.size() < WORKER_JOB_LIMIT) {
    //
    // }
}

auto Client::getOwnPort() const -> int {
    return mOwnServer.port();
}

auto Client::isBusy() const -> bool {
    return mJobQueue.size() >= WORKER_JOB_LIMIT;
}
