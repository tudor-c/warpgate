#include <argparse/argparse.hpp>

#include "warpgate.h"
#include "consts.h"
#include "log.h"
#include "tracker/tracker.h"
#include "client/client.h"

Warpgate::Warpgate(const int argc, const char* argv[]) :
    mArgc(argc), mArgv(argv) {}

auto Warpgate::run() -> int {
    lg::init();

    if (!parseArgs(mArgc, mArgv)) {
        return 1;
    }

    if (mIsTrackerSelected) {
        Tracker tracker;
        return tracker.run();
    }
    if (mIsClientSelected) {
        try {
            Client client(
                mTrackerHost,
                mTrackerPort,
                mNotWorker,
                mTaskConfigPath,
                mOutputPath);
            return client.run();
        }
        catch (const std::runtime_error& err) {
            lg::error(err.what());
        }
    }
    return 0;
}

auto Warpgate::parseArgs(const int argc, const char *argv[]) -> bool {
    argparse::ArgumentParser program("warpgate");

    const auto TRACKER_SUBPARSER = "tracker";
    const auto CLIENT_SUBPARSER = "client";

    argparse::ArgumentParser clientArgs(CLIENT_SUBPARSER);
    clientArgs.add_argument(FLAG_CLIENT_TRACKER_HOST)
        .default_value(TRACKER_HOST)
        .store_into(mTrackerHost)
        .help("Tracker host address");

    clientArgs.add_argument(FLAG_CLIENT_TRACKER_PORT)
        .default_value(TRACKER_PORT)
        .store_into(mTrackerPort)
        .help("Tracker port");

    clientArgs.add_argument(FLAG_CLIENT_TASK_CONFIG_PATH)
        .help("Path to task file")
        .store_into(mTaskConfigPath)
        .default_value("");
    clientArgs.add_argument(FLAG_CLIENT_OUTPUT_PATH)
        .help("Path where the output of the submitted task will be saved")
        .store_into(mOutputPath)
        .default_value(TASK_OUTPUT_DEFAULT_PATH);
    clientArgs.add_argument(FLAG_CLIENT_NOT_WORKER)
        .help("Node will only request a task but will not receive any work to do")
        .store_into(mNotWorker)
        .default_value(false);

    argparse::ArgumentParser trackerArgs(TRACKER_SUBPARSER);
    trackerArgs.add_argument(FLAG_TRACKER_PORT)
        .help("The port of the RPC server on the tracker that clients will connect to")
        .store_into(mPort)
        .default_value(8080);

    program.add_subparser(clientArgs);
    program.add_subparser(trackerArgs);
    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return false;
    }

    mIsTrackerSelected = program.is_subcommand_used(TRACKER_SUBPARSER);
    mIsClientSelected = program.is_subcommand_used(CLIENT_SUBPARSER);

    // additional conditions between flags
    if (mTaskConfigPath.empty() && mNotWorker) {
        lg::error("Client is neither a worker, nor is has submitted a task!");
        return false;
    }

    return true;
}
