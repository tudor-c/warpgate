#include <format>
#include <ranges>

#include "tracker.h"
#include "consts.h"
#include "Task.h"
#include "log.h"

Tracker::Tracker() : mRpcServer(TRACKER_PORT) {
    lg::info("Starting tracker at {}:{} 🚀",
        LOCALHOST, mRpcServer.port());

    mHeartbeatCheckThread = std::thread([this] {
        this->refreshClientListLoop();
    });

    this->bindRpcServerFunctions();
}

auto Tracker::run() -> int {
    mRpcServer.run();
    return 0;
}

auto Tracker::registerWorker(const std::string &host, int port) -> Id {
    // TODO check duplicates
    const auto id = util::generateUniqueId();
    const auto socketAddr = socketAddress(host, port);

    mClients[id] = Client {
        .socketAddr = socketAddr,
        .worker = std::make_unique<rpc::client>(host, port),
        .lastHeartbeat = std::chrono::system_clock::now(),
        .isFree = true};

    printWorkers();
    return id;
}

auto Tracker::unregisterWorker(const int id) -> void {
    mClients.erase(id);
}

auto Tracker::printWorkers() const -> void {
    std::string workersInfo = "Workers:";
    for (const auto& client: mClients | std::views::values) {
        workersInfo += std::format("\n  {}", client.socketAddr);
    }
    lg::debug(workersInfo);
}

auto Tracker::registerTask(const Task& task) -> void {
    const auto id = util::generateUniqueId();
    mTasks.insert({id, task});
    dispatchAvailableSubtasksByTask(id);
}

auto Tracker::dispatchAvailableSubtasksByTask(const Id taskId) -> void {
    auto subtasks = mTasks.at(taskId).getAvailableSubtasks();
    for (auto& subtask : subtasks) {
        // TODO --
    }
}

auto Tracker::socketAddress(const std::string &host, const int port) -> std::string {
    return std::format("{}:{}", host, std::to_string(port));
}

auto Tracker::bindRpcServerFunctions() -> void {
    mRpcServer.bind(RPC_REGISTER_CLIENT, [this](const std::string& host, const int port) {
        return this->registerWorker(host, port);
    });
    mRpcServer.bind(RPC_UNREGISTER_CLIENT, [this](const Id id) {
        this->unregisterWorker(id);
    });
    mRpcServer.bind(RPC_TEST_ANNOUNCEMENT, [this](const std::string& mess) {
        for (const auto& client : mClients | std::views::values) {
            client.worker->call(RPC_TEST_ANNOUNCEMENT_BROADCAST, mess);
        }
    });
    mRpcServer.bind(RPC_SUBMIT_TASK, [this](const Task& task) {
        task.printStructure();
    });
    mRpcServer.bind(RPC_HEARTBEAT, [this](const Id clientId) {
        this->refreshClientHeartbeat(clientId);
    });
}

auto Tracker::refreshClientListLoop() -> void { // TODO add end condition and thread joining
    while (true) {
        const auto now = std::chrono::system_clock::now();
        for (auto it = mClients.begin(); it != mClients.cend(); ) {
            auto& clientId = it->first;
            auto& lastHeartbeat = it->second.lastHeartbeat;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - lastHeartbeat).count() > CLIENT_HEARTBEAT_MAX_INTERVAL_MS) {
                lg::info("Removed client {} after no heartbeat", clientId);
                mClients.erase(it++);
            } else {
                ++it;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TRACKER_HEARTBEAT_CHECK_INTERVAL_MS));
    }
}

auto Tracker::refreshClientHeartbeat(const Id clientId) -> void {
    if (!mClients.contains(clientId)) {
        return;
    }
    mClients.at(clientId).lastHeartbeat = std::chrono::system_clock::now();
}


