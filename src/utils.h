#pragma once

#include <string>

// unique identifier type
using Id = int;

namespace util {
    auto readTextFile(const std::string &path) -> std::string;
    auto scheduleTask(int intervalMs,
        const std::function<void()>& fn, const std::function<bool()>& shouldEnd) -> void;
    auto getSocketAddress(const std::string& host, int port) -> std::string;
}
