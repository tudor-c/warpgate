#pragma once

// Suppress warning from external library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess" // TODO maybe remove
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
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
        std::chrono::time_point<std::chrono::system_clock> lastHeartbeat;
    };

private:
    std::unordered_map<int, Client> mRpcClients; // TODO lock behind RW guard
    rpc::server mRpcServer;
    std::thread mHeartbeatCheckThread;

    int generateNewClientId() const;

    void refreshClientList();

    void refreshClientHeartbeat(int clientId);
};
