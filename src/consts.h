#pragma once

// RPC Tracker function names
constexpr auto RPC_REGISTER_CLIENT = "registerClient";
constexpr auto RPC_UNREGISTER_CLIENT = "unregisterClient";
constexpr auto RPC_TEST_METHOD = "testMethod";
constexpr auto RPC_SEND_FILE_TEST = "sendFile";
constexpr auto RPC_SUBMIT_TASK = "submitTask";

// RPC Client function names
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
constexpr auto FLAG_CLIENT_TASK_CONFIG_PATH = "--task";

// JSON task config keys
constexpr auto JSON_TASK_NAME = "name";
constexpr auto JSON_SUBTASKS = "subtasks";
constexpr auto JSON_TASK_ID = "id";
constexpr auto JSON_TASK_ROOT = "root";
constexpr auto JSON_TASK_FUNCTION = "function";
constexpr auto JSON_TASK_DEPENDS_ON = "depends_on";

