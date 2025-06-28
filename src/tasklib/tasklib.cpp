#include <string>
#include <format>
#include <vector>

extern "C" std::string func1(const std::vector<std::string>& args) {
    return std::format("func1({}, {})", args[0], args[1]);
}

extern "C" std::string func2(std::vector<std::string>& args) {
    return std::format("func2({}, {})", args[0], args[1]);
}

extern "C" std::string func3(std::vector<std::string>& args) {
    return std::format("func3({})", args[0]);
}

extern "C" std::string func4(const std::vector<std::string>& args) {
    return std::format("func4: {}", args[0]);
}

extern "C" std::string func5(const std::vector<std::string>& args) {
    return std::format("func5: {}", args[0]);
}

extern "C" std::string func6(const std::vector<std::string>& args) {
    return std::format("func6: {}", args[0]);
}
