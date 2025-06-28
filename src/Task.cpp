#include <format>
#include <fstream>

#include <nlohmann/json.hpp>

#include "consts.h"
#include "Task.h"

#include "log.h"
#include "types.h"

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
        return std::ranges::any_of(keys, [&configEntry](const auto& key) {
            return !configEntry.contains(key);
        });
    };

    if (anyMissing(config, {JSON_TASK_NAME, JSON_TASK_ROOT,JSON_TASK_LIB_PATH, JSON_SUBTASKS})) {
        throw std::runtime_error("Missing config name, lib path or subtasks section!\n");
    }
    mName = config.at(JSON_TASK_NAME);
    mRootIndex = config.at(JSON_TASK_ROOT);
    mLibPath = config.at(JSON_TASK_LIB_PATH);

    for (auto entry : config.at(JSON_SUBTASKS)) {
        if (anyMissing(entry, {JSON_SUBTASK_INDEX, JSON_SUBTASK_FUNCTION, JSON_SUBTASK_DEPENDS_ON})) {
            throw std::runtime_error("Missing subtask fields!\n");
        }
        auto& index = entry.at(JSON_SUBTASK_INDEX);
        auto& functionName = entry.at(JSON_SUBTASK_FUNCTION);
        auto& dependsOn = entry.at(JSON_SUBTASK_DEPENDS_ON);
        auto inputPath = std::string{};
        if (entry.contains(JSON_SUBTASK_INPUT_PATH)) {
            inputPath = entry.at(JSON_SUBTASK_INPUT_PATH);
        }
        mSubtasks.emplace(index, Subtask {
            .index = index,
            .functionName = functionName,
            .inputDataPath = inputPath,
            .dependencyIndices = dependsOn,
            .dependencyIds = {},
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
        subtaskInfo += std::format(" - index: {}, name: {}, dependsOn: {}",
            subtask.index, subtask.functionName, subtask.dependencyIndices);
    }
    lg::debug("Task: {}, root: {}, subtasks:\n{}", mName, mRootIndex, subtaskInfo);
}

auto Task::getName() const -> std::string {
    return mName;
}

auto Task::getSubtasks() const -> std::unordered_map<Id, Subtask> {
    return mSubtasks;
}

auto Task::getLibPath() const -> std::string {
    return mLibPath;
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

auto Task::isCompleted() -> bool {
    return this->getRootSubtask().get().status == Subtask::COMPLETED;
}

auto Task::isSubtaskAvailable(const Subtask& subtask) const -> bool {
    return subtask.status == Subtask::AVAILABLE &&
        std::ranges::all_of(subtask.dependencyIndices, [this](const Id dependencyId) {
          return mSubtasks.at(dependencyId).status == Subtask::COMPLETED;
        });
}

auto Task::getRootSubtask() -> std::reference_wrapper<Subtask> {
    for (auto& subtask : mSubtasks | std::views::values) {
        if (subtask.index == mRootIndex) {
            return std::ref(subtask);
        }
    }
    return std::ref(mSubtasks.begin()->second);
}
