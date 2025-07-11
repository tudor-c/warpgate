#pragma once

// RPC Tracker function names
constexpr auto RPC_REGISTER_CLIENT = "registerClient";
constexpr auto RPC_UNREGISTER_CLIENT = "unregisterClient";
constexpr auto RPC_SEND_FILE_TEST = "sendFile";
constexpr auto RPC_SUBMIT_TASK_TO_TRACKER = "submitTaskToTracker";
constexpr auto RPC_HEARTBEAT = "heartbeat";
constexpr auto RPC_ANNOUNCE_SUBTASK_COMPLETED = "markSubtaskCompleted";
constexpr auto RPC_FETCH_JOB_COMPLETER_ADDRESS = "jobCompleterAddress";
constexpr auto RPC_FETCH_TASK_ACQUIRER_ADDRESS = "taskAcquirerAddress";
constexpr auto RPC_FETCH_SUBTASK_ACQUIRER_ADDRESS = "subtaskAcquirerAddress";
constexpr auto RPC_FETCH_SUBTASK_INPUT_DATA = "subtaskInputData";

// RPC Client function names
constexpr auto RPC_DISPATCH_JOB = "dispatchJob";
constexpr auto RPC_FETCH_SUBTASK_RESULT = "fetchSubtaskResult";
constexpr auto RPC_FETCH_LIB_CONTENT = "fetchLibContent";
constexpr auto RPC_ANNOUNCE_TASK_COMPLETED = "announceTaskCompleted";

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
constexpr auto FLAG_CLIENT_OUTPUT_PATH = "--output";
constexpr auto FLAG_CLIENT_NOT_WORKER = "--not-worker";
constexpr auto FLAG_CLIENT_JOB_LIMIT = "--job-limit";

constexpr auto FLAG_TRACKER_PORT = "--port";

// JSON task config keys
constexpr auto JSON_TASK_NAME = "name";
constexpr auto JSON_TASK_LIB_PATH = "lib";
constexpr auto JSON_TASK_ROOT = "root";
constexpr auto JSON_SUBTASKS = "subtasks";
constexpr auto JSON_SUBTASK_INDEX = "index";
constexpr auto JSON_SUBTASK_FUNCTION = "function";
constexpr auto JSON_SUBTASK_INPUT_PATH = "input_path";
constexpr auto JSON_SUBTASK_DEPENDS_ON = "depends_on";

constexpr int CLIENT_HEARTBEAT_PERIOD_MS = 500;
constexpr int CLIENT_HEARTBEAT_MAX_INTERVAL_MS = 1000;
constexpr int CLIENT_JOB_LAUNCH_INTERVAL_MS = 100;
constexpr int TRACKER_HEARTBEAT_CHECK_INTERVAL_MS = 50;
constexpr int TRACKER_DISPATCH_SUBTASKS_INTERVAL_MS = 200;

// Max numbers of jobs a worker can take at the same time
constexpr int ACTIVE_JOB_LIMIT = 1;

// to use with std::format with taskId as field
constexpr auto TEMP_LIB_FILENAME_TEMPLATE = "lib_task_{}";

constexpr auto TASK_OUTPUT_DEFAULT_PATH = "output.data";
