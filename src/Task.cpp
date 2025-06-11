#include <format>
#include <fstream>

#include <nlohmann/json.hpp>

#include "consts.h"
#include "Task.h"

#include "log.h"

using json = nlohmann::json;

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
    mRootIndex = config.at(JSON_TASK_ROOT);

    for (auto entry : config.at(JSON_SUBTASKS)) {
        if (anyMissing(entry, {JSON_TASK_INDEX, JSON_TASK_FUNCTION, JSON_TASK_DEPENDS_ON})) {
            throw std::runtime_error("Missing subtask fields!\n");
        }
        auto& index = entry.at(JSON_TASK_INDEX);
        auto& functionName = entry.at(JSON_TASK_FUNCTION);
        auto& dependsOn = entry.at(JSON_TASK_DEPENDS_ON);
        mSubtasks.emplace(index, Subtask {
            .index = index,
            .functionName = functionName,
            .dependsOn = dependsOn,
            .status = Subtask::AVAILABLE});
    }
}

auto Task::printStructure() const -> void {
    std::string subtaskInfo;
    bool first = true;
    for (const auto &subtask : mSubtasks | std::views::values) {
        if (!first) {
            subtaskInfo += "\n";
        }
        first = false;
        subtaskInfo += std::format(" - id: {}, name: {}, dependsOn: {}",
            subtask.index, subtask.functionName, subtask.dependsOn);
    }
    lg::debug("Task: {}, root: {}, subtasks:\n{}", mName, mRootIndex, subtaskInfo);
}

auto Task::getAvailableSubtasks() -> std::vector<std::reference_wrapper<Subtask>> {
    return mSubtasks
        | std::views::values
        | std::views::filter([this](const Subtask &subtask) {
            return this->isSubtaskAvailable(subtask);
        })
        | std::ranges::to<std::vector<std::reference_wrapper<Subtask>>>();
}

auto Task::getAllSubtasks() -> std::vector<std::reference_wrapper<Subtask>> {
    return mSubtasks
        | std::views::values
        | std::ranges::to<std::vector<std::reference_wrapper<Subtask>>>();
}

auto Task::isCompleted() const -> bool {
    return mSubtasks.at(mRootIndex).status;
}

auto Task::isSubtaskAvailable(const Subtask& subtask) const -> bool {
    return subtask.status == Subtask::AVAILABLE &&
        std::ranges::all_of(subtask.dependsOn, [this](const Id dependencyId) {
          return mSubtasks.at(dependencyId).status == Subtask::COMPLETED;
        });
}
