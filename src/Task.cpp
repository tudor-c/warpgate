#include <format>
#include <fstream>

#include <nlohmann/json.hpp>

#include "consts.h"
#include "Task.h"

#include "log.h"

using json = nlohmann::json;

Task::Task(): mRoot(0) {}

Task::Task(const std::string& path) {
    json config;
    if (std::ifstream file(path); file.is_open()) {
        config = json::parse(file);
    }
    else {
        throw std::runtime_error(std::format("No such task config file: {}\n", path));
    }

    auto anyMissing = [&](const json& configEntry, const std::initializer_list<std::string_view>& keys) {
        for (auto& key : keys) {
            if (!configEntry.contains(key)) {
                return true;
            }
        }
        return false;
    };

    if (anyMissing(config, {JSON_TASK_NAME, JSON_TASK_ROOT, JSON_SUBTASKS})) {
        throw std::runtime_error("Missing config name or subtasks section!\n");
    }
    mName = config.at(JSON_TASK_NAME);
    mRoot = config.at(JSON_TASK_ROOT);

    for (auto entry : config.at(JSON_SUBTASKS)) {
        if (anyMissing(entry, {JSON_TASK_ID, JSON_TASK_FUNCTION, JSON_TASK_DEPENDS_ON})) {
            throw std::runtime_error("Missing subtask fields!\n");
        }
        auto id = entry.at(JSON_TASK_ID);
        auto functionName = entry.at(JSON_TASK_FUNCTION);
        auto dependsOn = entry.at(JSON_TASK_DEPENDS_ON);
        mSubtasks[id] = Subtask { id, false, functionName, dependsOn };
    }
}

Task::Task(const Task &other) :
    mName(other.mName), mRoot(other.mRoot), mSubtasks(other.mSubtasks) {}

void Task::printStructure() const {
    std::string subtaskInfo;
    bool first = true;
    for (const auto &subtask : mSubtasks | std::views::values) {
        if (!first) {
            subtaskInfo += "\n";
        }
        first = false;
        subtaskInfo += std::format(" - id: {}, name: {}, dependsOn: {}",
            subtask.id, subtask.functionName, subtask.dependsOn);
    }
    lg::debug("Task: {}, root: {}, subtasks:\n{}", mName, mRoot, subtaskInfo);
}

std::vector<Subtask> Task::getAvailableSubtasks() const {
    return mSubtasks
        | std::views::values
        | std::views::filter([this](const Subtask& task) {
            return !task.completed && std::ranges::all_of(task.dependsOn, [this](const auto& dependencyId) {
                return mSubtasks.at(dependencyId).completed;
            });
        })
        | std::ranges::to<std::vector<Subtask>>();
}

bool Task::isCompleted() const {
    return mSubtasks.at(mRoot).completed;
}
