#pragma once

#include <string>

struct SocketAddress {
    std::string host;
    int port;

    std::string toString() const {
        return std::format("{}:{}", host, port);
    }
};
