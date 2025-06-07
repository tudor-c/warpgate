#pragma once

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

    ~Client();

    auto run() -> int;

private:
    auto registerAsClient() -> bool;
    auto startHeartbeatThread() -> std::thread;
    auto getOwnPort() const -> int;

    auto unregisterAsClient() -> void;
    auto registerTask(const Task &) -> void;

    auto teardown() -> void;

    Id mOwnId = -1;  // unique id assigned by host on client creation

    const std::string mTrackerHost;
    const int mTrackerPort;

    rpc::client mTrackerConn;
    rpc::server mOwnServer;

    std::thread mServerThread;
    std::thread mHeartbeatThread;
};
