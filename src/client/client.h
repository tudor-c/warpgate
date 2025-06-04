#pragma once

#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess" // TODO maybe remove
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <rpc/client.h>
#include <rpc/rpc_error.h>
#include <rpc/server.h>

#include "Task.h"
#pragma GCC diagnostic pop


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

    int getOwnPort() const;

    int mOwnId = -1;  // unique id assigned by host on client creation

    std::string mTrackerHost;
    int mTrackerPort;

    rpc::client mClient;

    rpc::server mOwnServer;
    std::thread mServerThread;
};
