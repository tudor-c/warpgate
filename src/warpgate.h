#pragma once

#include <string>

class Warpgate {
public:
    Warpgate(int argc, const char* argv[]);

    Warpgate(const Warpgate&) = delete;

    auto run() -> int;

private:
    auto parseArgs(int argc, const char *argv[]) -> bool;

    const int mArgc;
    const char **mArgv;

    bool mIsTrackerSelected = false;
    bool mIsClientSelected = false;

    // client options
    std::string mTrackerHost;
    int mTrackerPort = 0;
    std::string mTaskConfigPath;
    std::string mOutputPath;
    bool mTrackerTestArg = false;
    bool mNotWorker = false;
    int mJobLimit = 1;

    // tracker options
    int mPort = 8080;
};
