#pragma once

#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#include <rpc/client.h>
#include <rpc/rpc_error.h>
#include <rpc/server.h>
#pragma GCC diagnostic pop


class Client {

public:
    Client(const std::string& trackerHost, int trackerPort);

    ~Client();

    int run();

    bool registerAsWorker();

private:

    void unregisterAsWorker();

    int getOwnPort() const;

    int mOwnId = -1;  // unique id assigned by host on client creation

    std::string mTrackerHost;
    int mTrackerPort;

    rpc::client mClient;

    rpc::server mOwnServer;
    std::thread mServerThread;
};
