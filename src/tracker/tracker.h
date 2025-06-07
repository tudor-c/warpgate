#pragma once

#include <rpc/client.h>
#include <rpc/server.h>

#include "types.h"

class Tracker {

public:
    Tracker();

    auto run() -> int;

    auto registerWorker(const std::string &host, int port) -> ClientId;
    auto unregisterWorker(ClientId id) -> void;
    auto printWorkers() const -> void;

    static auto socketAddress(const std::string &host, int port) -> std::string;

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

    auto bindRpcServerFunctions() -> void;
    auto generateNewClientId() const -> ClientId;

    auto refreshClientListLoop() -> void;
    auto refreshClientHeartbeat(ClientId clientId) -> void;

};
