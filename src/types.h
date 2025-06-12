#pragma once

#include <string>

// unique identifier type
using Id = int;

// type returned by subtasks as result and received as parameters
using ResultType = std::string;

struct SocketAddress {
    std::string host;
    int port;

    std::string toString() const {
        return std::format("{}:{}", host, port);
    }

    MSGPACK_DEFINE(host, port);
};
