#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <rpc/msgpack.hpp>

#include "utils.h"


struct Subtask {
    enum Status { AVAILABLE, ENQUEUED, SUBMITTED, COMPLETED};


    Id id = -1;
    std::string functionName;
    std::vector<Id> dependsOn;
    Status status = AVAILABLE;

    MSGPACK_DEFINE(id, functionName, dependsOn, status)
};

MSGPACK_ADD_ENUM(Subtask::Status);


class Task {
public:
    Task();
    Task(const std::string& path);
    Task(const Task& other);

    auto printStructure() const -> void;
    auto getAvailableSubtasks(bool markAsEnqueued) -> std::vector<Subtask>;
    auto isCompleted() const -> bool;

    MSGPACK_DEFINE(mName, mRoot, mSubtasks)

private:
    std::string mName;
    Id mRoot = 0;
    std::unordered_map<Id, Subtask> mSubtasks;
};
