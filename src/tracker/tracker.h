#pragma once

// Suppress warning from external library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess" // TODO maybe remove
#include <rpc/client.h>
#include <rpc/server.h>
#pragma GCC diagnostic pop

class Tracker {

public:
    Tracker();

    int run();

    int registerWorker(const std::string& host, int port);

    void unregisterWorker(int id);

    void printWorkers() const;

    static std::string socketAddress(std::string host, int port);

    struct Client {
        std::string socketAddr;
        std::unique_ptr<rpc::client> worker;
    };

private:
    std::unordered_map<int, Client> mRpcClients;
    rpc::server mRpcServer;

    int generateNewClientId() const;
};
