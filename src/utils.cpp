#include <fstream>
#include <filesystem>

#include "utils.h"

namespace util {
    auto readTextFile(const std::string &path) -> std::string {
        const auto size = file_size(std::filesystem::path(path));
        std::string buffer(size, '\0');
        std::ifstream file(path);
        file.read(&buffer[0], size);
        return buffer;
    }

    auto generateUniqueId() -> Id {
        static std::atomic_int currentId;
        return currentId++;
    }

}
