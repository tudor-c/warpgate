#pragma once

#include <string>

class Warpgate {
public:
    Warpgate(int argc, const char* argv[]);

    Warpgate(const Warpgate&) = delete;

    int run();

private:
    bool parseArgs(int argc, const char* argv[]);

    int mArgc;
    const char **mArgv;

    bool isTrackerSelected = false;
    bool isClientSelected = false;

    // client options
    std::string mTrackerHost = "";
    int mTrackerPort = 0;
    std::string mTaskConfigPath = "";
    bool mTrackerTestArg = false;
};