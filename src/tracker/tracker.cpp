#include <format>
#include <ranges>

#include "tracker.h"
#include "consts.h"
#include "Task.h"
#include "log.h"

Tracker::Tracker() : mRpcServer(TRACKER_PORT) {
    lg::info("Starting tracker at {}:{} 🚀",
        LOCALHOST, mRpcServer.port());
    this->bindRpcServerFunctions();
}

auto Tracker::run() -> int {
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
            [this] { this->updateJobQueue(); },
            [this] { return false; } // TODO implement end condition
        );
    });
    mRpcServer.run();
    return 0;
}

auto Tracker::registerWorker(const std::string &host, int port) -> Id {
    // TODO check duplicates
    const auto id = this->generateUniqueId();

    mClients[id] = Client {
        .id = id,
        .socketAddress = SocketAddress {host, port},
        .rpcClient = std::make_unique<rpc::client>(host, port),
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
        workersInfo += std::format("\n  {}", client.socketAddress.toString());
    }
    lg::debug(workersInfo);
}

auto Tracker::registerTask(const Task& task, const Id acquirerId) -> void {
    const auto taskId = this->generateUniqueId();
    mTasks.insert({taskId, task});
    mTaskAcquirers.insert({taskId, acquirerId});
    lg::debug("Assigned id {} to task.", taskId);

    std::unordered_map<int, Id> indexToId;

    // assign taskID and unique Ids to all subtasks
    for (const auto& subtask : mTasks.at(taskId).getAllSubtasks()) {
        const auto id = this->generateUniqueId();
        subtask.get().id = id;
        subtask.get().taskId = taskId;
        indexToId.insert({subtask.get().index, id});
        mAllSubtasks.insert({id, std::ref(subtask)});
    }

    // replace dependsOn index values with newly assigned unique IDs
    for (const auto& subtaskRef : mTasks.at(taskId).getAllSubtasks()) {
        auto& subtask = subtaskRef.get();
        subtask.dependencyIds = subtask.dependencyIndices
            | std::views::transform([indexToId](const int index) {
                return indexToId.at(index);
            })
            | std::ranges::to<std::vector<Id>>();
    }
}

auto Tracker::updateJobQueue() -> void {
    this->enqueueAvailableJobs();
    this->dispatchJobsFromQueue();
    this->announceFinishedTasks();
}

auto Tracker::enqueueAvailableJobs() -> void {
    for (auto& task : mTasks | std::views::values) {
        for (auto& subtask : task.getAvailableSubtasks()) {
            mSubtaskQueue.push(subtask);
            subtask.get().status = Subtask::ENQUEUED;
        }
    }
}

auto Tracker::dispatchJobsFromQueue() -> void {
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
            if (nextWorker == std::ranges::end(availableWorkers)) {
                break;
            }
            auto& worker = *nextWorker;
            accepted = worker.rpcClient->call(RPC_DISPATCH_JOB, subtask).as<bool>();
            if (accepted) {
                lg::debug("Subtask {} accepted by worker {}.", subtask.functionName, worker.id);
                subtask.status = Subtask::SUBMITTED;
                worker.isFree = false;
                mSubtaskQueue.pop();
            }
            else {
                lg::debug("Subtask {} NOT accepted by worker {}!",
                    subtask.functionName, worker.id);
            }
            ++nextWorker;
        }
    }
}

auto Tracker::markSubtaskCompleted(const Id workerId, const Id subtaskId) -> void {
    mClients.at(workerId).isFree = true;
    auto& subtask = mAllSubtasks.at(subtaskId).get();
    subtask.completedBy = workerId;
    subtask.status = Subtask::COMPLETED;
    lg::debug("Subtask {} completed by {}.", mAllSubtasks.at(subtaskId).get().functionName,
        subtask.completedBy);
}

auto Tracker::announceFinishedTasks() -> void {
    for (auto it = mTasks.begin(); it != mTasks.end();) {
        auto& taskId = it->first;
        auto& task = it->second;
        if (!task.isCompleted()) {
            ++it;
            continue;
        }
        const auto& rootSubtask = task.getRootSubtask();
        const auto& acquirerId = mTaskAcquirers.at(taskId);
        lg::debug("Announcing finished task {} by {}",
            rootSubtask.get().functionName, rootSubtask.get().completedBy);
        const auto& completerAddress = mClients.at(rootSubtask.get().completedBy)
            .socketAddress;
        mClients.at(acquirerId).rpcClient->call(
            RPC_ANNOUNCE_TASK_COMPLETED, rootSubtask.get().id, completerAddress);
        mTasks.erase(it++);
    }
}

auto Tracker::getJobCompleterSocketAddress(const Id subtaskId) const -> SocketAddress {
    const auto workerId = mAllSubtasks.at(subtaskId).get().completedBy;
    return mClients.at(workerId).socketAddress;
}

auto Tracker::getTaskAcquirerSockerAddress(const Id taskId) const -> SocketAddress {
    const auto clientId = mTaskAcquirers.at(taskId);
    auto res = mClients.at(clientId).socketAddress;
    return res;
}

auto Tracker::bindRpcServerFunctions() -> void {
    mRpcServer.bind(RPC_REGISTER_CLIENT, [this](const std::string& host, const int port) {
        return this->registerWorker(host, port);
    });
    mRpcServer.bind(RPC_UNREGISTER_CLIENT, [this](const Id clientId) {
        this->unregisterWorker(clientId);
    });
    mRpcServer.bind(RPC_SUBMIT_TASK_TO_TRACKER, [this](const Task& task,
        const Id acquirerId) {
        task.printStructure();
        this->registerTask(task, acquirerId);
    });
    mRpcServer.bind(RPC_HEARTBEAT, [this](const Id clientId) {
        this->refreshClientHeartbeat(clientId);
    });
    mRpcServer.bind(RPC_ANNOUNCE_SUBTASK_COMPLETED, [this](const Id workerId, const Id subtaskId) {
        this->markSubtaskCompleted(workerId, subtaskId);
    });
    mRpcServer.bind(RPC_FETCH_JOB_COMPLETER_ADDRESS, [this](const Id subtaskId) {
        return this->getJobCompleterSocketAddress(subtaskId);
    });
    mRpcServer.bind(RPC_FETCH_TASK_ACQUIRER_ADDRESS, [this](const Id taskId) {
        return this->getTaskAcquirerSockerAddress(taskId);
    });
}

auto Tracker::refreshClientList() -> void {
    const auto now = std::chrono::system_clock::now();
    for (auto it = mClients.begin(); it != mClients.cend(); ) {
        const auto timeSinceLastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second.lastHeartbeat).count();
        if (timeSinceLastHeartbeat > CLIENT_HEARTBEAT_MAX_INTERVAL_MS) {
            lg::info("Removed client {} after no heartbeat.", it->first);
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


