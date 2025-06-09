#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <rpc/msgpack.hpp>

#include "utils.h"


struct Subtask {
    enum Status { AVAILABLE, ENQUEUED, SUBMITTED, COMPLETED};

    // index, as specified in json config file, not unique between tasks
    int index;
    // unique id assigned by the tracker, not present in json representation
    Id id = -1;
    // id of the worker that completed this subtask
    Id completedBy = -1;
    // function name of subtask's entry point
    std::string functionName;
    // indexes of dependency subtasks
    std::vector<int> dependsOn;
    // status of the subtask, maintained by the tracker
    Status status = AVAILABLE;

    // Subtask(const Subtask& other);

    MSGPACK_DEFINE(index, id, completedBy, functionName, dependsOn, status)
};

MSGPACK_ADD_ENUM(Subtask::Status);


class Task {
public:
    Task();
    Task(const std::string& path);
    Task(const Task& other);

    auto printStructure() const -> void;
    auto getAvailableSubtasks() -> std::vector<std::reference_wrapper<Subtask>>;
    auto getAllSubtasks() -> std::vector<std::reference_wrapper<Subtask>>;
    auto isCompleted() const -> bool;

    MSGPACK_DEFINE(mName, mRootIndex, mSubtasks)

private:
    std::string mName;
    int mRootIndex = 0;
    std::unordered_map<Id, Subtask> mSubtasks;
};
