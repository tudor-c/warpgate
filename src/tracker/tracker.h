#pragma once

#include <queue>
#include <rpc/client.h>
#include <rpc/server.h>

#include "Task.h"
#include "utils.h"

class Tracker {

public:
    Tracker();

    auto run() -> int;

    struct Client {
        int id;
        std::string socketAddr;
        std::unique_ptr<rpc::client> client;
        std::chrono::time_point<std::chrono::system_clock> lastHeartbeat;
        bool isWorker;
        bool isFree;
    };

private:
    std::unordered_map<Id, Client> mClients; // TODO lock behind RW guard
    rpc::server mRpcServer;
    std::unordered_map<Id, Task> mTasks;

    // all subtasks from all tasks
    std::unordered_map<Id, std::reference_wrapper<Subtask>> mAllSubtasks;
    std::queue<std::reference_wrapper<Subtask>> mSubtaskQueue;
    std::thread mHeartbeatCheckThread;
    std::thread mSubtaskDispatchThread;

    std::atomic_int mCurrentUniqueId = 1;

    static auto socketAddress(const std::string &host, int port) -> std::string;

    auto bindRpcServerFunctions() -> void;

    auto registerWorker(const std::string &host, int port) -> Id;
    auto unregisterWorker(Id id) -> void;
    auto registerTask(const Task&) -> void;
    auto printWorkers() const -> void;

    auto updateJobQueue() -> void;
    auto enqueueAvailableJobs() -> void;
    auto dispatchJobsFromQueue() -> void;
    auto markSubtaskCompleted(Id workerId, Id subtaskId) -> void;

    auto refreshClientList() -> void;
    auto refreshClientHeartbeat(Id clientId) -> void;

    auto generateUniqueId() -> Id;
};
