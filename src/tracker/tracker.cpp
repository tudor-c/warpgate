#include <format>
#include <iostream>
#include <ranges>
#include <print>

#include "tracker.h"
#include "consts.h"
#include "Task.h"

Tracker::Tracker() : mRpcServer(TRACKER_PORT) {
    std::cout << std::format("\nStarting tracker at {}:{} 🚀\n\n",
        LOCALHOST, mRpcServer.port());

    mHeartbeatCheckThread = std::thread([this] {
        this->refreshClientList();
    });

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
    mRpcServer.bind(RPC_SUBMIT_TASK, [this](const Task& task) {
        task.printStructure();
    });
    mRpcServer.bind(RPC_HEARTBEAT, [this](const int clientId) {
        this->refreshClientHeartbeat(clientId);
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
        std::make_unique<rpc::client>(host, port),
        std::chrono::system_clock::now()};

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

void Tracker::refreshClientList() {
    while (true) {
        const auto now = std::chrono::system_clock::now();
        for (auto it = mRpcClients.begin(); it != mRpcClients.cend(); ) {
            auto& clientId = it->first;
            auto& lastHeartbeat = it->second.lastHeartbeat;

            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - lastHeartbeat).count() > CLIENT_HEARTBEAT_MAX_INTERVAL_MS) {
                mRpcClients.erase(it++);
                std::print("Removed client {} after no heartbeat\n", clientId);
            }
            else {
                ++it;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TRACKER_HEARTBEAT_CHECK_INTERVAL_MS));
    }
}

void Tracker::refreshClientHeartbeat(int clientId) {
    if (!mRpcClients.contains(clientId)) {
        return;
    }
    mRpcClients.at(clientId).lastHeartbeat = std::chrono::system_clock::now();
}

