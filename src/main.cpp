#include "warpgate.h"

auto main(const int argc, const char *argv[]) -> int {
    Warpgate warpgate(argc, argv);
    return warpgate.run();
}