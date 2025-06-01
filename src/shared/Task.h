#pragma once

#include <string>
#include <vector>

struct SubTask {
    int id;
    bool completed = false;
    std::string functionName;
    std::vector<int> dependsOn;
};


class Task {
public:
    Task(const std::string& path);

    std::vector<SubTask> getAvailableSubtasks() const;

    bool isCompleted() const;

private:
    SubTask rootTask;

};
