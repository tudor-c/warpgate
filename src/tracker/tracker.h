#pragma once
#include <rpc/client.h>
#include <rpc/server.h>

class Tracker {

public:
    Tracker();

    void run();

    int registerWorker(const std::string& host, int port);

    void unregisterWorker(int id);

    void printWorkers() const;

    static std::string socketAddress(std::string host, int port);

    struct Client {
        std::string socketAddr;
        std::unique_ptr<rpc::client> worker;
    };

private:
    std::unordered_map<int, Client> mClients;
    rpc::server mServer;

    int generateNewClientId() const;
};
