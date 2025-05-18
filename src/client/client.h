#pragma once
#include <string>
#include <rpc/client.h>
#include <rpc/rpc_error.h>
#include <rpc/server.h>

class Client {

public:
    Client(const std::string& trackerHost, int trackerPort);

    ~Client();

    void run();

    bool registerAsWorker();

private:

    void unregisterAsWorker();

    int getOwnPort() const;

    std::string mTrackerHost;
    int mTrackerPort;
    rpc::server mOwnServer;
    rpc::client mClient;
};
