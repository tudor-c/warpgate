#include <format>
#include <iostream>

#include "client.h"
#include "consts.h"
#include "../Task.h"

Client::Client(
    const std::string &trackerHost,
    const int trackerPort,
    [[maybe_unused]] bool registerAsWorker, // TODO use
    const std::string &taskConfigPath) :
        mTrackerHost(trackerHost),
        mTrackerPort(trackerPort),
        mClient(trackerHost, trackerPort),
        mOwnServer(FREE_PORT) {

    std::cout << std::format("\nStarting worker at {}:{} 🌐\n\n",
        LOCALHOST, mOwnServer.port());

    if (!taskConfigPath.empty()) {
        try {
            Task task(taskConfigPath);
            task.print();
        }
        catch (std::runtime_error& e) {
            std::cerr << e.what() << '\n';
        }
    }
}

Client::~Client() {
    unregisterAsClient();
}

int Client::run() {
    if (!registerAsClient()) {
        std::cerr << "Could not connect!\n";
        return 1;
    }

    mOwnServer.bind(RPC_TEST_ANNOUNCEMENT_BROADCAST, [](const std::string& mess) {
        std::cout << "Announcement from another peer: " << mess << "\n";
    });
    mServerThread = std::thread(&rpc::server::run, &mOwnServer);

    mClient.call(RPC_TEST_ANNOUNCEMENT,
        std::format("Hello world! I'm {}:{}\n", LOCALHOST, mOwnServer.port()));

    teardown();
    return 0;
}

bool Client::registerAsClient() {
    mClient.set_timeout(TIMEOUT_MS);
    try {
        mOwnId = mClient.call(RPC_REGISTER_CLIENT, LOCALHOST, getOwnPort()).as<int>();
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

void Client::teardown() {
    mServerThread.join();
    unregisterAsClient();
}

void Client::unregisterAsClient() {
    std::cout << std::format("Unregistered as worker for tracker at {}:{}\n",
        mTrackerHost, mTrackerPort);
    mClient.call(RPC_UNREGISTER_CLIENT, mOwnId);
}

int Client::getOwnPort() const {
    return mOwnServer.port();
}
