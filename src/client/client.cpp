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
        mOwnServer(FREE_PORT) {

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
            registerTask(*task);
        }
    }
}

Client::~Client() {
    unregisterAsClient();
}

auto Client::run() -> int {
    if (!registerAsClient()) {
        lg::error("Could not connect!");
        return 1;
    }

    mServerThread = std::thread(&rpc::server::run, &mOwnServer);
    mHeartbeatThread = startHeartbeatThread();
    mTrackerConn.call(RPC_TEST_ANNOUNCEMENT,
        std::format("Hello world! I'm {}:{}\n", LOCALHOST, mOwnServer.port()));

    teardown();
    return 0;
}

auto Client::bindRcpServerFunctions() -> void {
    mOwnServer.bind(RPC_TEST_ANNOUNCEMENT_BROADCAST, [](const std::string& mess) {
        lg::debug("Announcement from another peer: {}", mess);
    });
    mOwnServer.bind(RPC_DISPATCH_SUBTASK, [this](const Subtask& subtask) {
        return this->receiveSubtask(subtask);
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

auto Client::registerTask(const Task &task) -> void {
    mTrackerConn.call(RPC_SUBMIT_TASK, task);
}

auto Client::receiveSubtask(const Subtask& subtask) -> bool  {
    lg::debug("Accepted subtask: {}", subtask.functionName);
    return true;
}

auto Client::startHeartbeatThread() -> std::thread {
    return std::thread([this] {
        while (true) {
            mTrackerConn.call(RPC_HEARTBEAT, mOwnId);
            std::this_thread::sleep_for(std::chrono::milliseconds(CLIENT_HEARTBEAT_PERIOD_MS));
        }
    });
}

auto Client::getOwnPort() const -> int {
    return mOwnServer.port();
}
