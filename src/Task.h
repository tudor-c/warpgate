#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <rpc/msgpack.hpp>

#include "types.h"


struct Subtask {
    SubtaskId id = -1;
    std::string functionName;
    std::vector<SubtaskId> dependsOn;
    bool completed = false;

    MSGPACK_DEFINE(id, functionName, dependsOn, completed)
};


class Task {
public:
    Task();
    Task(const std::string& path);
    Task(const Task& other);

    auto printStructure() const -> void;
    auto getAvailableSubtasks() const -> std::vector<Subtask>;
    auto isCompleted() const -> bool;

    MSGPACK_DEFINE(mName, mRoot, mSubtasks)

private:
    std::string mName;
    SubtaskId mRoot = 0;
    std::unordered_map<SubtaskId, Subtask> mSubtasks;
};
