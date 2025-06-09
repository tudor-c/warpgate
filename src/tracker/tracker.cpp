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
    const auto id = this->generateUniqueId();
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

auto Tracker::registerTask(const Task& task) -> void {
    const auto taskId = this->generateUniqueId();
    mTasks.insert({taskId, task});

    // assign unique Ids to all subtasks
    for (const auto& subtask : mTasks.at(taskId).getAllSubtasks()) {
        const auto id = this->generateUniqueId();
        subtask.get().id = id;
        mAllSubtasks.insert({id, std::ref(subtask)});
    }

    // enqueue newly available subtasks
    for (auto& subtask : mTasks.at(taskId).getAvailableSubtasks()) {
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
            }
            else {
                lg::debug("Subtask {} NOT accepted by worker {}", subtask.functionName, worker.id);
            }
            ++nextWorker;
        }
    }
}

auto Tracker::markSubtaskCompleted(const Id workerId, const Id subtaskId) -> void {
    mClients.at(workerId).isFree = true;
    auto& subtask = mAllSubtasks.at(subtaskId).get();
    subtask.status = Subtask::COMPLETED;
    subtask.completedBy = workerId;
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
    mRpcServer.bind(RPC_ANNOUNCE_SUBTASK_COMPLETED, [this](const Id workerId, const Id subtaskId) {
        this->markSubtaskCompleted(workerId, subtaskId);
    });
}

auto Tracker::refreshClientList() -> void {
    const auto now = std::chrono::system_clock::now();
    for (auto it = mClients.begin(); it != mClients.cend(); ) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                now - it->second.lastHeartbeat).count() > CLIENT_HEARTBEAT_MAX_INTERVAL_MS) {
            lg::info("Removed client {} after no heartbeat", it->first);
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

auto Tracker::generateUniqueId() -> Id {
    return mCurrentUniqueId++;
}


