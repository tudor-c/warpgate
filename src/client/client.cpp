    #include "client.h"

#include <format>
#include <iostream>

#include "consts.h"

Client::Client(const std::string &trackerHost, int trackerPort) :
        mTrackerHost(trackerHost),
        mTrackerPort(trackerPort),
        mClient(trackerHost, trackerPort),
        mOwnServer(FREE_PORT) {
    std::cout << std::format("\nStarting worker at {}:{} 🌐\n\n",
        LOCALHOST, mOwnServer.port());
}

Client::~Client() {
    unregisterAsWorker();
}

int Client::run() {
    bool connected = registerAsWorker();
    if (!connected) {
        std::cerr << "Could not connect!\n";
        return 1;
    }

    mOwnServer.bind(RPC_TEST_ANNOUNCEMENT_BROADCAST, [](const std::string& mess) {
        std::cout << "Announcement from another peer: " << mess << "\n";
    });
    mServerThread = std::thread(&rpc::server::run, &mOwnServer);

    mClient.call(RPC_TEST_ANNOUNCEMENT,
        std::format("Hello world! I'm {}:{}\n", LOCALHOST, mOwnServer.port()));

    // teardown
    mServerThread.join();
    unregisterAsWorker();
    return 0;
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
