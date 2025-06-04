#include <argparse/argparse.hpp>

#include "warpgate.h"
#include "consts.h"
#include "tracker/tracker.h"
#include "client/client.h"

Warpgate::Warpgate(int argc, const char* argv[]) :
    mArgc(argc), mArgv(argv) {}


int Warpgate::run() {
    if (!parseArgs(mArgc, mArgv)) {
        return 1;
    }

    if (mIsTrackerSelected) {
        Tracker tracker;
        return tracker.run();
    }
    if (mIsClientSelected) {
        // can be a worker only if it isn't an Acquirer TODO add flag for this
        bool registerAsWorker = mTaskConfigPath.empty();
        Client client(
            mTrackerHost,
            mTrackerPort,
            registerAsWorker,
            mTaskConfigPath);
        return client.run();
    }
    return 0;
}

bool Warpgate::parseArgs(int argc, const char* argv[]) {
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

    argparse::ArgumentParser trackerArgs(TRACKER_SUBPARSER);
    trackerArgs.add_argument("--test")
        .store_into(mTrackerTestArg)
        .default_value(false)
        .implicit_value(true);

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

    return true;
}
