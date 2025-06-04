#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <rpc/msgpack.hpp>


struct Subtask {
    int id;
    bool completed = false;
    std::string functionName;
    std::vector<int> dependsOn;

    MSGPACK_DEFINE(id, completed, functionName, dependsOn)
};


class Task {
public:
    Task();
    Task(const std::string& path);
    Task(const Task& other);

    void printStructure() const;

    std::vector<Subtask> getAvailableSubtasks() const;

    bool isCompleted() const;

    MSGPACK_DEFINE(mName, mRoot, mSubtasks)

private:
    std::string mName;
    int mRoot;
    std::unordered_map<int, Subtask> mSubtasks;
};
