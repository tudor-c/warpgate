#include <format>
#include <iostream>
#include <ranges>

#include "tracker.h"
#include "consts.h"

Tracker::Tracker() : mServer(TRACKER_PORT) {
    std::cout << std::format("\nStarting tracker at {}:{} 🚀\n\n",
        LOCALHOST, mServer.port());

    mServer.bind(RPC_REGISTER_CLIENT, [this](const std::string &host, int port) {
        return this->registerWorker(host, port);
    });
    mServer.bind(RPC_UNREGISTER_CLIENT, [this](int id) {
        this->unregisterWorker(id);
    });
    mServer.bind(RPC_TEST_METHOD, [] {
        std::cout << "test method called\n";
    });
    mServer.bind(RPC_TEST_ANNOUNCEMENT, [this](const std::string& mess) {
        for (const auto& client : mClients | std::views::values) {
            client.worker->call(RPC_TEST_ANNOUNCEMENT_BROADCAST, mess);
        }
    });
}

int Tracker::run() {
    mServer.run();
    return 0;
}

int Tracker::registerWorker(const std::string &host, int port) {
    // TODO check duplicates
    int id = generateNewClientId();
    const std::string socketAddr = socketAddress(host, port);

    mClients[id] = Client {
        socketAddr,
        std::make_unique<rpc::client>(host, port)
    };
    printWorkers();
    return id;
}

void Tracker::unregisterWorker(int id) {
    mClients.erase(id);
}

void Tracker::printWorkers() const {
    std::cout << "Workers:\n";
    for (const auto& client: mClients | std::views::values) {
        auto& socketAddr = client.socketAddr;
        std::cout << std::format("  {}\n", socketAddr);
    }
}

std::string Tracker::socketAddress(std::string host, int port) {
    return std::format("{}:{}", host, std::to_string(port));
}

int Tracker::generateNewClientId() const {
    int id = -1;
    while (id == -1 || mClients.contains(id)) {
        id = std::rand();
    }
    return id;
}

