#include <format>
#include <fstream>

#include <nlohmann/json.hpp>

#include "Task.h"
#include "consts.h"

using json = nlohmann::json;

Task::Task(const std::string& path) {
    json config;
    if (std::ifstream file(path); file.is_open()) {
        config = json::parse(file);
    }
    else {
        throw std::runtime_error(std::format("No such task config file: {}\n", path));
    }

    if (!config.contains(JSON_TASK_NAME) || !config.contains(JSON_SUBTASKS)) {
        throw std::runtime_error("Missing config name or subtasks section!\n");
    }

    mName = config.at(JSON_TASK_NAME);
    for (auto entry : config.at(JSON_SUBTASKS)) {
        if (!entry.contains(JSON_TASK_ID) ||
            !entry.contains(JSON_TASK_FUNCTION) ||
            !entry.contains(JSON_TASK_DEPENDS_ON)) {
            throw std::runtime_error("Missing subtask fields!\n");
        }
        auto id = entry.at(JSON_TASK_ID);
        auto functionName = entry.at(JSON_TASK_FUNCTION);
        auto dependsOn = entry.at(JSON_TASK_DEPENDS_ON);
        mSubtasks[id] = Subtask { id, false, functionName, dependsOn };
    }
}

std::vector<Subtask> Task::getAvailableSubtasks() const {
}

bool Task::isCompleted() const {
    return rootTask.completed;
}
