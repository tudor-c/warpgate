#pragma once

// RPC function names
constexpr auto RPC_REGISTER_WORKER = "registerWorker";
constexpr auto RPC_UNREGISTER_WORKER = "unregisterWorker";
constexpr auto RPC_TEST_METHOD = "testMethod";

// Connection parameters
constexpr auto TRACKER_HOST = "127.0.0.1";
constexpr auto TRACKER_PORT = 8080;
constexpr auto LOCALHOST = "127.0.0.1";
constexpr auto FREE_PORT = 0;

constexpr auto TIMEOUT_MS = 3000;