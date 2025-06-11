#include <fstream>
#include <filesystem>

#include "utils.h"

namespace util {
    auto readTextFile(const std::string &path) -> std::string {
        const auto size = file_size(std::filesystem::path(path));
        std::string buffer(size, '\0');
        std::ifstream file(path);
        file.read(&buffer[0], static_cast<long>(size));
        return buffer;
    }

    auto scheduleTask(const int intervalMs,
        const std::function<void()>& fn, const std::function<bool()>& shouldEnd) -> void {
        while (!shouldEnd()) {
            fn();
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    }

    auto getSocketAddress(const std::string &host, const int port) -> std::string {
        return std::format("{}:{}", host, std::to_string(port));
    }
}
