#include <format>

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
    auto taskPath = args.get<std::string>("--task");




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
