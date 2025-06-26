#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <rpc/msgpack.hpp>

#include "types.h"
#include "utils.h"


struct Subtask {
    enum Status { AVAILABLE, ENQUEUED, SUBMITTED, COMPLETED};

    // index, as specified in json config file, not unique between tasks
    int index = -1;
    // unique id assigned by the tracker, not present in json representation
    Id id = -1;
    // the unique id of the containing task, as assigned bt the tracker
    Id taskId = -1;
    // id of the worker that completed this subtask
    Id completedBy = -1;
    // function name of subtask's entry point
    std::string functionName;
    // indexes of dependency subtasks
    std::vector<int> dependencyIndices;
    // unique ids of dependency subtasks, assigned after subtask is registered by tracker
    std::vector<Id> dependencyIds;
    // status of the subtask, maintained by the tracker
    Status status = AVAILABLE;

    MSGPACK_DEFINE(index, id, taskId, completedBy, functionName, dependencyIndices, dependencyIds, status)
};

MSGPACK_ADD_ENUM(Subtask::Status);


class Task {
public:
    Task() = default;
    Task(const Task& other) = default;
    explicit Task(const std::string& path);

    auto printStructure() const -> void;
    auto getAvailableSubtasks() -> std::vector<std::reference_wrapper<Subtask>>;
    auto getAllSubtasks() -> std::vector<std::reference_wrapper<Subtask>>;
    auto isCompleted() const -> bool;
    auto isSubtaskAvailable(const Subtask&) const -> bool;
    auto getRootSubtask() -> std::reference_wrapper<Subtask>;

    MSGPACK_DEFINE(mName, mRootIndex, mSubtasks)

private:
    std::string mName;
    int mRootIndex = 0;
    std::unordered_map<Id, Subtask> mSubtasks;
};
