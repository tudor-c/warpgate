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
        util::scheduleTask(
            TRACKER_HEARTBEAT_CHECK_INTERVAL_MS,
            [this] { this->refreshClientList(); },
            [this] { return false; } // TODO implement end condition
        );
    });
    mSubtaskDispatchThread = std::thread([this] {
        util::scheduleTask(
            TRACKER_DISPATCH_SUBTASKS_INTERVAL_MS,
            [this] { this->dispatchSubtasksFromQueue(); },
            [this] { return false; } // TODO implement end condition
        );
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
        .id = id,
        .socketAddr = socketAddr,
        .client = std::make_unique<rpc::client>(host, port),
        .lastHeartbeat = std::chrono::system_clock::now(),
        .isWorker = true, // TODO add condition
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

auto Tracker::registerTask(Task task) -> void {
    const auto taskId = util::generateUniqueId();
    mTasks.insert({taskId, task});

    for (auto& subtask : mTasks.at(taskId).getAvailableSubtasks(true)) {
        mSubtaskQueue.push(subtask);
    }
}

auto Tracker::dispatchSubtasksFromQueue() -> void {
    auto availableWorkers = mClients
        | std::views::values
        | std::views::filter([](const auto& client) {
            return client.isWorker && client.isFree;
        });
    auto nextWorker = std::ranges::begin(availableWorkers);

    while (!mSubtaskQueue.empty()) {
        auto& subtask = mSubtaskQueue.front().get();
        if (nextWorker == std::ranges::end(availableWorkers)) {
            break; // no other free workers
        }
        bool accepted = false;
        while (!accepted) {
            auto& worker = *nextWorker;
            accepted = worker.client->call(RPC_DISPATCH_SUBTASK, subtask).as<bool>();
            if (accepted) {
                lg::debug("Subtask {} accepted by worker {}", subtask.functionName, worker.id);
                subtask.status = Subtask::SUBMITTED;
                worker.isFree = false;
                mSubtaskQueue.pop();
                ++nextWorker;
                break;
            }

            lg::debug("Subtask {} NOT accepted by worker {}", subtask.functionName, worker.id);
            ++nextWorker;
        }
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
            client.client->call(RPC_TEST_ANNOUNCEMENT_BROADCAST, mess);
        }
    });
    mRpcServer.bind(RPC_SUBMIT_TASK, [this](const Task& task) {
        task.printStructure();
        this->registerTask(task);
    });
    mRpcServer.bind(RPC_HEARTBEAT, [this](const Id clientId) {
        this->refreshClientHeartbeat(clientId);
    });
}

auto Tracker::refreshClientList() -> void {
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
}

auto Tracker::refreshClientHeartbeat(const Id clientId) -> void {
    if (!mClients.contains(clientId)) {
        return;
    }
    mClients.at(clientId).lastHeartbeat = std::chrono::system_clock::now();
}


