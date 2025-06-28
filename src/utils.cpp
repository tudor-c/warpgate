#include <fstream>
#include <filesystem>

#include "utils.h"

namespace util {
    auto readBinaryFile(const std::string &path) -> std::vector<unsigned char> {
        std::filesystem::path inputFilePath(path);
        auto len = std::filesystem::file_size(inputFilePath);
        std::vector<unsigned char> buffer(len);
        std::ifstream file(path, std::ios::binary);
        file.read(reinterpret_cast<char*>(buffer.data()), len);
        return buffer;
    }

    auto readTextFile(const std::string &path) -> std::string {
        std::ifstream file(path);
        return std::string((std::istreambuf_iterator(file)),
            std::istreambuf_iterator<char>());
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
