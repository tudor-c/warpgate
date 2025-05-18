#include <format>
#include <iostream>

#include "tracker.h"
#include "consts.h"

Tracker::Tracker() : mServer(TRACKER_PORT) {
    std::cout << "Starting tracker 🚀\n";

    mServer.bind(RPC_REGISTER_WORKER, [this](const std::string &host, int port) {
        this->registerWorker(host, port);
    });
    mServer.bind(RPC_UNREGISTER_WORKER, [this](const std::string &host, int port) {
        this->unregisterWorker(host, port);
    });
    mServer.bind(RPC_TEST_METHOD, [] {
        std::cout << "test method called\n";
    });
}

void Tracker::run() {
    mServer.run();
}

void Tracker::registerWorker(const std::string &host, int port) {
    // TODO check duplicates
    mWorkers[socketAddress(host, port)] = std::make_unique<rpc::client>(host, port);
    printWorkers();
}

void Tracker::unregisterWorker(const std::string& host, int port) {
    std::erase_if(mWorkers, [&](const auto& pair) {
        return pair.first == socketAddress(host, port);
    });
}

void Tracker::printWorkers() const {
    std::cout << "Workers:\n";
    for (auto&[socketAddress, _] : mWorkers) {
        std::cout << std::format("  {}\n", socketAddress);
    }
}

std::string Tracker::socketAddress(std::string host, int port) {
    return std::format("{}:{}", host, std::to_string(port));
}

