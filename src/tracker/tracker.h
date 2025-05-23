#pragma once
#include <rpc/client.h>
#include <rpc/server.h>

class Tracker {

public:
    Tracker();

    void run();

    void registerWorker(const std::string& host, int port);

    void unregisterWorker(const std::string& host, int port);

    void printWorkers() const;

    static std::string socketAddress(std::string host, int port);

private:
    std::unordered_map<std::string, std::unique_ptr<rpc::client>> mWorkers;
    rpc::server mServer;
};
