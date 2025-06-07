#pragma once

#include <rpc/client.h>
#include <rpc/server.h>

#include "types.h"

class Tracker {

public:
    Tracker();

    int run();

    ClientId registerWorker(const std::string& host, int port);

    void unregisterWorker(ClientId id);

    void printWorkers() const;

    static std::string socketAddress(const std::string& host, int port);

    struct Client {
        std::string socketAddr;
        std::unique_ptr<rpc::client> worker;
        std::chrono::time_point<std::chrono::system_clock> lastHeartbeat;
        bool isFree;
    };

private:
    std::unordered_map<ClientId, Client> mClients; // TODO lock behind RW guard
    rpc::server mRpcServer;
    std::thread mHeartbeatCheckThread;

    ClientId generateNewClientId() const;

    void refreshClientList();

    void refreshClientHeartbeat(ClientId clientId);

    void bindRpcServerFunctions();
};
