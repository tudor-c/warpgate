#include <format>
#include <dlfcn.h>

#include <argparse/argparse.hpp>

#include "client.h"
#include "consts.h"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser args("client");

    args.add_argument("--port")
        .help("Tracker port")
        .default_value(TRACKER_PORT)
        .scan<'i', int>();

    args.add_argument("--task")
        .help("Path to task file");

    try {
        args.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << args;
        return 1;
    }

    int trackerPort = args.get<int>("--port");
    auto tasklibPath = args.get<std::string>("--task");

    system("pwd");
    std::cout << std::format("|{}|\n", tasklibPath);
    void* handle = dlopen(tasklibPath.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cout << "nu merge..\n";
    }
    std::cout << "se incarca!\n";
    typedef void (*hello_func)();
    hello_func hello = (hello_func)dlsym(handle, "test_fun");
    hello();



    // Client client(LOCALHOST, 8080);
    // bool connected = client.registerAsWorker();
    // if (!connected) {
    //     std::cerr << "Could not connect!\n";
    //     return 1;
    // }
    //
    // sleep(50);
    //
    // return 0;
}
