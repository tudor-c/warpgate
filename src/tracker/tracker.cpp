#include <format>
#include <ranges>

#include "tracker.h"
#include "consts.h"
#include "Task.h"
#include "log.h"

Tracker::Tracker() : mRpcServer(TRACKER_PORT) {
    lg::info("Starting tracker at {}:{} 🚀",
        LOCALHOST, mRpcServer.port());

    mHeartbeatCheckThread = std::thread([this] {
        this->refreshClientList();
    });

    this->bindRpcServerFunctions();
}

int Tracker::run() {
    mRpcServer.run();
    return 0;
}

ClientId Tracker::registerWorker(const std::string &host, int port) {
    // TODO check duplicates
    const auto id = generateNewClientId();
    const auto socketAddr = socketAddress(host, port);

    mClients[id] = Client {
        .socketAddr = socketAddr,
        .worker = std::make_unique<rpc::client>(host, port),
        .lastHeartbeat = std::chrono::system_clock::now(),
        .isFree = true};

    printWorkers();
    return id;
}

void Tracker::unregisterWorker(const int id) {
    mClients.erase(id);
}

void Tracker::printWorkers() const {
    std::string workersInfo = "Workers:";
    for (const auto& client: mClients | std::views::values) {
        workersInfo += std::format("\n  {}", client.socketAddr);
    }
    lg::debug(workersInfo);
}

std::string Tracker::socketAddress(const std::string& host, const int port) {
    return std::format("{}:{}", host, std::to_string(port));
}

ClientId Tracker::generateNewClientId() const {
    ClientId id = -1;
    while (id == -1 || mClients.contains(id)) {
        id = std::rand();
    }
    return id;
}

void Tracker::refreshClientList() {
    while (true) {
        const auto now = std::chrono::system_clock::now();
        for (auto it = mClients.begin(); it != mClients.cend(); ) {
            auto& clientId = it->first;
            auto& lastHeartbeat = it->second.lastHeartbeat;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - lastHeartbeat).count() > CLIENT_HEARTBEAT_MAX_INTERVAL_MS) {
                lg::info("Removed client {} after no heartbeat", clientId);
                mClients.erase(it++);
            } else {
                ++it;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TRACKER_HEARTBEAT_CHECK_INTERVAL_MS));
    }
}

void Tracker::refreshClientHeartbeat(const ClientId clientId) {
    if (!mClients.contains(clientId)) {
        return;
    }
    mClients.at(clientId).lastHeartbeat = std::chrono::system_clock::now();
}

void Tracker::bindRpcServerFunctions() {
    mRpcServer.bind(RPC_REGISTER_CLIENT, [this](const std::string& host, const int port) {
        return this->registerWorker(host, port);
    });
    mRpcServer.bind(RPC_UNREGISTER_CLIENT, [this](const ClientId id) {
        this->unregisterWorker(id);
    });
    mRpcServer.bind(RPC_TEST_ANNOUNCEMENT, [this](const std::string& mess) {
        for (const auto& client : mClients | std::views::values) {
            client.worker->call(RPC_TEST_ANNOUNCEMENT_BROADCAST, mess);
        }
    });
    mRpcServer.bind(RPC_SUBMIT_TASK, [this](const Task& task) {
        task.printStructure();
    });
    mRpcServer.bind(RPC_HEARTBEAT, [this](const ClientId clientId) {
        this->refreshClientHeartbeat(clientId);
    });
}

