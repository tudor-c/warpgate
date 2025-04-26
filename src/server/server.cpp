#include <iostream>

#include "crow.h"

int main() {
    std::cout << "Hello world from Server!\n";

    std::vector<std::string> clientHosts;

    crow::SimpleApp app;
    CROW_ROUTE(app, "/")([] {
        return "hello world";
    });
    CROW_ROUTE(app, "/post").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        auto jsonBody = crow::json::load(req.body);
        auto val = jsonBody["arg"].i();
        std::cout << val << '\n';
        return "merge";
    });

    app.port(18080).multithreaded().run();
}