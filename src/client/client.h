#pragma once

#include <queue>
#include <string>

#include <rpc/client.h>
#include <rpc/server.h>

#include "Task.h"
#include "utils.h"

class Client {

public:
    Client(
        const std::string& trackerHost,
        int trackerPort,
        bool registerAsWorker,
        const std::string &taskConfigPath);

    ~Client() = default;

    auto run() -> int;

private:
    auto bindRcpServerFunctions() -> void;

    auto registerAsClient() -> bool;
    auto unregisterAsClient() -> void;
    auto getOwnPort() const -> int;

    auto isBusy() const -> bool;
    auto submitTaskToTracker(const Task &) -> void;
    auto receiveJob(const Subtask& subtask) -> bool;
    auto processJobsQueues() -> void;
    auto launchJobsFromQueue() -> void;
    auto sendFinishedJobsNotifications() -> void;

    auto teardown() -> void;

    Id mOwnId = -1;  // unique id assigned by host on client creation

    const std::string mTrackerHost;
    const int mTrackerPort;

    rpc::client mTrackerConn;
    rpc::server mOwnServer;

    std::thread mServerThread;
    std::thread mHeartbeatThread;
    std::thread mJobThread;

    std::queue<Subtask> mJobQueue;
    // pairs of finished subtask id and its result
    // TODO change result type from string to actual data
    std::queue<std::pair<Id, std::string>> mJobResults;
    // results of completed subtasks, indexed by subtask id
    std::unordered_map<Id, std::string> mResults;
    // currently running threads, indexed by subtask id
    std::unordered_map<Id, std::thread> mWorkerThreads;
};
