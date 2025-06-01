#include "Task.h"

Task::Task(const std::string& path) {
}

std::vector<SubTask> Task::getAvailableSubtasks() const {
}

bool Task::isCompleted() const {
    return rootTask.completed;
}
