#include <format>

#include <argparse/argparse.hpp>

#include "client.h"
#include "consts.h"

bool parseArguments(int argc, char* argv[], argparse::ArgumentParser& args) {
    args.add_argument(FLAG_CLIENT_TRACKER_HOST)
        .default_value(TRACKER_HOST)
        .help("Tracker host address");

    args.add_argument(FLAG_CLIENT_TRACKER_PORT)
        .default_value(TRACKER_PORT)
        .scan<'i', int>()
        .help("Tracker port");

    args.add_argument(FLAG_CLIENT_TASK_CONFIG_PATH)
        .help("Path to task file");

    try {
        args.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << args;
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser args("client");
    if (!parseArguments(argc, argv, args)) {
        return 1;
    }

    auto trackerHost = args.get<std::string>("--host");
    auto trackerPort = args.get<int>("--port");
    auto taskConfigPath = args.get<std::string>("--task");

    Client client(
        LOCALHOST,
        trackerPort,
        true,
        taskConfigPath);

    return client.run();
}
