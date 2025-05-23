#pragma once
#include <string>
#include <rpc/client.h>
#include <rpc/rpc_error.h>
#include <rpc/server.h>

class Client {

public:
    Client(const std::string& trackerHost, int trackerPort);

    ~Client();

    int run();

    bool registerAsWorker();

private:

    void unregisterAsWorker();

    int getOwnPort() const;

    int mAssignedId;  // unique id assigned by host on client creation

    std::string mTrackerHost;
    int mTrackerPort;

    rpc::client mClient;

    rpc::server mOwnServer;
    std::thread mServerThread;
};
