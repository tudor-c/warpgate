#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

constexpr auto LOG_LEVEL = spdlog::level::debug;

namespace lg {
    inline std::shared_ptr<spdlog::logger> logger;

    inline auto init() -> void {
        logger = spdlog::stdout_color_mt("warpgate");
        logger->set_pattern("%^[%H:%M:%S] [%l]%$ %v");
        logger->set_level(LOG_LEVEL);
        spdlog::set_default_logger(logger);
    }

    template<typename... Args>
    inline auto info(fmt::format_string<Args...> fmt, Args &&... args) -> void {
        logger->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline auto warn(fmt::format_string<Args...> fmt, Args &&... args) -> void {
        logger->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline auto error(fmt::format_string<Args...> fmt, Args &&... args) -> void {
        logger->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline auto debug(fmt::format_string<Args...> fmt, Args &&... args) -> void {
        logger->debug(fmt, std::forward<Args>(args)...);
    }

    inline auto info(const std::string &msg) -> void {
        logger->info(msg);
    }

    inline auto warn(const std::string &msg) -> void {
        logger->warn(msg);
    }

    inline auto error(const std::string &msg) -> void {
        logger->error(msg);
    }

    inline auto debug(const std::string &msg) -> void {
        logger->debug(msg);
    }
}