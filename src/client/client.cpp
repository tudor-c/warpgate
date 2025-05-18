#include "client.h"

#include <format>
#include <iostream>

#include "consts.h"

Client::Client(const std::string &trackerHost, int trackerPort) :
        mTrackerHost(trackerHost),
        mTrackerPort(trackerPort),
        mOwnServer(FREE_PORT),
        mClient(trackerHost, trackerPort) {
    std::cout << "Starting worker 🌐\n";
    // mOwnServer.run();
}

Client::~Client() {
    unregisterAsWorker();
}

void Client::run() {
    // TODO implement
}

bool Client::registerAsWorker() {
    mClient.set_timeout(TIMEOUT_MS);
    try {
        mClient.call(RPC_REGISTER_WORKER, LOCALHOST, getOwnPort());
    } catch (std::exception& e) {
        std::cerr << std::format("Could not connect to tracker at {}:{}!\n {}\n",
            mTrackerHost, mTrackerPort, e.what());
        return false;
    }
    mClient.clear_timeout();

    std::cout << std::format("Registered as worker for tracker at {}:{}\n",
        mTrackerHost, mTrackerPort);
    mClient.call(RPC_TEST_METHOD);

    return true;
}

void Client::unregisterAsWorker() {
    std::cout << std::format("Unregistered as worker for tracker at {}:{}\n",
        mTrackerHost, mTrackerPort);
    mClient.call(RPC_UNREGISTER_WORKER, LOCALHOST, getOwnPort()); // TODO replace with some ID
}

int Client::getOwnPort() const {
    return mOwnServer.port();
}
