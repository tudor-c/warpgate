#include <format>
#include <iostream>
#include <ranges>

#include "tracker.h"
#include "consts.h"

Tracker::Tracker() : mRpcServer(TRACKER_PORT) {
    std::cout << std::format("\nStarting tracker at {}:{} 🚀\n\n",
        LOCALHOST, mRpcServer.port());

    mRpcServer.bind(RPC_REGISTER_CLIENT, [this](const std::string &host, int port) {
        return this->registerWorker(host, port);
    });
    mRpcServer.bind(RPC_UNREGISTER_CLIENT, [this](int id) {
        this->unregisterWorker(id);
    });
    mRpcServer.bind(RPC_TEST_METHOD, [] {
        std::cout << "test method called\n";
    });
    mRpcServer.bind(RPC_TEST_ANNOUNCEMENT, [this](const std::string& mess) {
        for (const auto& client : mRpcClients | std::views::values) {
            client.worker->call(RPC_TEST_ANNOUNCEMENT_BROADCAST, mess);
        }
    });
}

int Tracker::run() {
    mRpcServer.run();
    return 0;
}

int Tracker::registerWorker(const std::string &host, int port) {
    // TODO check duplicates
    int id = generateNewClientId();
    const std::string socketAddr = socketAddress(host, port);

    mRpcClients[id] = Client {
        socketAddr,
        std::make_unique<rpc::client>(host, port)
    };
    printWorkers();
    return id;
}

void Tracker::unregisterWorker(int id) {
    mRpcClients.erase(id);
}

void Tracker::printWorkers() const {
    std::cout << "Workers:\n";
    for (const auto& client: mRpcClients | std::views::values) {
        auto& socketAddr = client.socketAddr;
        std::cout << std::format("  {}\n", socketAddr);
    }
}

std::string Tracker::socketAddress(std::string host, int port) {
    return std::format("{}:{}", host, std::to_string(port));
}

int Tracker::generateNewClientId() const {
    int id = -1;
    while (id == -1 || mRpcClients.contains(id)) {
        id = std::rand();
    }
    return id;
}

