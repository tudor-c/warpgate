#include "utils.h"

#include <fstream>
#include <sstream>
#include <filesystem>

auto readTextFile(const std::string &path) -> std::string {
    const auto size = file_size(std::filesystem::path(path));
    std::string buffer(size, '\0');
    std::ifstream file(path);
    file.read(&buffer[0], size);
    return buffer;
}
