#pragma once

#include <string>
#include <vector>

struct Subtask {
    int id;
    bool completed = false;
    std::string functionName;
    std::vector<int> dependsOn;
};


class Task {
public:
    Task(const std::string& path);

    std::vector<Subtask> getAvailableSubtasks() const;

    bool isCompleted() const;

private:
    std::string mName;
    Subtask rootTask;
    std::unordered_map<int, Subtask> mSubtasks;
};
