#pragma once

#include <string>

#include <rpc/client.h>
#include <rpc/server.h>

#include "Task.h"

class Client {

public:
    Client(
        const std::string& trackerHost,
        int trackerPort,
        bool registerAsWorker,
        const std::string &taskConfigPath);

    ~Client();

    int run();

    bool registerAsClient();

private:

    void teardown();

    void unregisterAsClient();

    void registerTask(const Task&);

    std::thread startHeartbeatThread();

    int getOwnPort() const;

    int mOwnId = -1;  // unique id assigned by host on client creation

    std::string mTrackerHost;
    int mTrackerPort;

    rpc::client mClient;

    rpc::server mOwnServer;
    std::thread mServerThread;
    std::thread mHeartbeatThread;
};
