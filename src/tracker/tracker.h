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
    std::queue<std::reference_wrapper<Subtask>> mSubtaskQueue;
    std::thread mHeartbeatCheckThread;
    std::thread mSubtaskDispatchThread;

    static auto socketAddress(const std::string &host, int port) -> std::string;

    auto bindRpcServerFunctions() -> void;

    auto registerWorker(const std::string &host, int port) -> Id;
    auto unregisterWorker(Id id) -> void;
    auto printWorkers() const -> void;

    auto registerTask(Task) -> void;
    auto dispatchSubtasksFromQueue() -> void;

    auto refreshClientList() -> void;
    auto refreshClientHeartbeat(Id clientId) -> void;
};
