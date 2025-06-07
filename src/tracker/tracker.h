#pragma once

#include <rpc/client.h>
#include <rpc/server.h>

#include "Task.h"
#include "utils.h"

class Tracker {

public:
    Tracker();

    auto run() -> int;

    struct Client {
        std::string socketAddr;
        std::unique_ptr<rpc::client> worker;
        std::chrono::time_point<std::chrono::system_clock> lastHeartbeat;
        bool isFree;
    };

private:
    std::unordered_map<Id, Client> mClients; // TODO lock behind RW guard
    rpc::server mRpcServer;
    std::thread mHeartbeatCheckThread;
    std::unordered_map<Id, Task> mTasks;

    static auto socketAddress(const std::string &host, int port) -> std::string;

    auto bindRpcServerFunctions() -> void;

    auto registerWorker(const std::string &host, int port) -> Id;
    auto unregisterWorker(Id id) -> void;
    auto printWorkers() const -> void;

    auto registerTask(const Task&) -> void;
    auto dispatchAvailableSubtasksByTask(Id taskId) -> void;

    auto refreshClientListLoop() -> void;
    auto refreshClientHeartbeat(Id clientId) -> void;
};
