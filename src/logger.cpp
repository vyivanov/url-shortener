#include "logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <filesystem>
#include <thread>
#include <memory>
#include <vector>

namespace {

constexpr uint16_t LOG_QUEUE_SIZE = (8192);
constexpr uint64_t LOG_FILE_SIZE  = (10*1024*1024);
constexpr uint64_t LOG_FILE_NUMS  = (10);

constexpr const char* LOG_NAME    = "logger";
constexpr const char* LOG_PATTERN = "[%Y-%m-%d %I:%M:%S %p] [%^%l%$] %v (thr-%t)";

}

namespace Shortener {

Logger& Logger::instance() noexcept {
    static Logger obj{"app.log"};
    return obj;
}

Logger::Logger(const std::filesystem::path& file) noexcept: m_logger(init(file)) {

}

Logger::logger_t Logger::init(const std::filesystem::path& file) noexcept {
    spdlog::init_thread_pool(LOG_QUEUE_SIZE, std::thread::hardware_concurrency());
    auto log_to_stdout = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto log_to_file   = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file, LOG_FILE_SIZE, LOG_FILE_NUMS);
    auto log_outputs   = std::vector<spdlog::sink_ptr>{log_to_stdout, log_to_file};
    auto logger        = std::make_shared<spdlog::async_logger>(LOG_NAME,
        log_outputs.begin()  ,
        log_outputs.end()    ,
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    log_to_stdout->set_pattern(LOG_PATTERN);
    log_to_file  ->set_pattern(LOG_PATTERN);
    return logger;
}

}
