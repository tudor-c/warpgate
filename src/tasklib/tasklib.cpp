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

extern "C" std::string func4([[maybe_unused]] const std::vector<std::string>& args) {
    return "func4";
}

extern "C" std::string func5([[maybe_unused]] const std::vector<std::string>& args) {
    return "func5";
}

extern "C" std::string func6([[maybe_unused]] const std::vector<std::string>& args) {
    return "func6";
}


// extern "C" void func1() {
//     std::cout << "func1\n";
// }
//
// extern "C" void func2() {
//     std::cout << "func2\n";
// }
//
// extern "C" void func3() {
//     std::cout << "func3\n";
// }
//
// extern "C" void func4() {
//     std::cout << "func4\n";
// }
//
// extern "C" void func5() {
//     std::cout << "func5\n";
// }
//
// extern "C" void func6() {
//     std::cout << "func6\n";
// }
//
