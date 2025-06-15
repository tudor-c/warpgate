#include <string>
#include <format>
#include <iostream>

// extern "C" std::string func1(std::string arg1, std::string arg2) {
//     return std::format("func1({}, {})", arg1, arg2);
// }
//
// extern "C" std::string func2(std::string arg1, std::string arg2) {
//     return std::format("func2({}, {})", arg1, arg2);
// }
//
// extern "C" std::string func3(std::string arg1) {
//     return std::format("func3({})", arg1);
// }
//
// extern "C" std::string func4() {
//     return "func4";
// }
//
// extern "C" std::string func5() {
//     return "func5";
// }
//
// extern "C" std::string func6() {
//     return "func6";
// }
//

extern "C" void func1() {
    std::cout << "func1\n";
}

extern "C" void func2() {
    std::cout << "func2\n";
}

extern "C" void func3() {
    std::cout << "func3\n";
}

extern "C" void func4() {
    std::cout << "func4\n";
}

extern "C" void func5() {
    std::cout << "func5\n";
}

extern "C" void func6() {
    std::cout << "func6\n";
}

