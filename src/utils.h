#pragma once

#include <string>

using Id = int;

namespace util {
    auto readTextFile(const std::string &path) -> std::string;
    auto generateUniqueId() -> Id;
    auto scheduleTask(int intervalMs,
        const std::function<void()>& fn, const std::function<bool()>& shouldEnd) -> void;
}
