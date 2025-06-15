#pragma once

#include <string>

namespace util {
    auto readBinaryFile(const std::string& path) -> std::vector<unsigned char>;
    auto scheduleTask(int intervalMs,
                      const std::function<void()>& fn, const std::function<bool()>& shouldEnd) -> void;
    auto getSocketAddress(const std::string& host, int port) -> std::string;
}
