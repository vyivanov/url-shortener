#pragma once

#include <spdlog/spdlog.h>

#include <utility>
#include <filesystem>
#include <memory>

namespace Shortener {

class Logger final {
public:
    static Logger& instance() noexcept;
public:
    template<typename... Args>
    void info(const char* const fmt, Args&&... args) noexcept {
        m_logger->info(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void warn(const char* const fmt, Args&&... args) noexcept {
        m_logger->warn(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void erro(const char* const fmt, Args&&... args) noexcept {
        m_logger->error(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void crit(const char* const fmt, Args&&... args) noexcept {
        m_logger->critical(fmt, std::forward<Args>(args)...);
    }
private:
    explicit Logger(const std::filesystem::path& file) noexcept;
    Logger(const Logger& other) = delete;
    Logger(Logger&& other) = delete;
    Logger& operator=(const Logger& other) = delete;
    Logger& operator=(Logger&& other) = delete;
   ~Logger() = default;
private:
    using  logger_t = std::shared_ptr<spdlog::logger>;
    static logger_t init(const std::filesystem::path& file) noexcept;
private:
    const logger_t m_logger;
};

}

template<typename... Args>
inline void LOG_INFO(const char* const fmt, Args&&... args) noexcept {
    Shortener::Logger::instance().info(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void LOG_WARNING(const char* const fmt, Args&&... args) noexcept {
    Shortener::Logger::instance().warn(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void LOG_ERROR(const char* const fmt, Args&&... args) noexcept {
    Shortener::Logger::instance().erro(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void LOG_CRITICAL(const char* const fmt, Args&&... args) noexcept {
    Shortener::Logger::instance().crit(fmt, std::forward<Args>(args)...);
}
