#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <rpc/msgpack.hpp>
#pragma GCC diagnostic pop


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
