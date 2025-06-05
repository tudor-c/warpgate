#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

constexpr auto LOG_LEVEL = spdlog::level::debug;

namespace lg {
    inline std::shared_ptr<spdlog::logger> logger;

    inline void init() {
        logger = spdlog::stdout_color_mt("warpgate");
        logger->set_pattern("%^[%H:%M:%S] [%l]%$ %v");
        logger->set_level(LOG_LEVEL);
        spdlog::set_default_logger(logger);
    }

    template<typename... Args>
    inline void info(fmt::format_string<Args...> fmt, Args&&... args) {
        logger->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        logger->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(fmt::format_string<Args...> fmt, Args&&... args) {
        logger->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        logger->debug(fmt, std::forward<Args>(args)...);
    }

    inline void info(const std::string& msg) {
        logger->info(msg);
    }

    inline void warn(const std::string& msg) {
        logger->warn(msg);
    }

    inline void error(const std::string& msg) {
        logger->error(msg);
    }

    inline void debug(const std::string& msg) {
        logger->debug(msg);
    }
}