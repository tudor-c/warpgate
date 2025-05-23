#pragma once

// RPC function names
//   Tracker
constexpr auto RPC_REGISTER_WORKER = "registerWorker";
constexpr auto RPC_UNREGISTER_WORKER = "unregisterWorker";
constexpr auto RPC_TEST_METHOD = "testMethod";

//   Client
constexpr auto RPC_TEST_ANNOUNCEMENT = "testAnnounce";
constexpr auto RPC_TEST_ANNOUNCEMENT_BROADCAST = "testAnnounceBroadcast";

// Connection parameters
constexpr auto LOCALHOST = "127.0.0.1";
constexpr auto TRACKER_HOST = LOCALHOST;
constexpr auto TRACKER_PORT = 8080;
constexpr auto FREE_PORT = 0;

constexpr auto TIMEOUT_MS = 3000;

// Command line flag names
constexpr auto FLAG_CLIENT_TRACKER_HOST = "--host";
constexpr auto FLAG_CLIENT_TRACKER_PORT = "--port";
constexpr auto FLAG_CLIENT_TASK_PATH = "--task";