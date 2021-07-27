#include "application.h"

#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>

#include <boost/format.hpp>

#include <cstdlib>
#include <exception>

namespace {

constexpr uint64_t LOG_FILE_SIZE = 10 * 1024 * 1024;
constexpr uint64_t LOG_FILE_NUMS = 10;

constexpr uint16_t APP_PORT_HTTP = 9080;
constexpr uint16_t APP_PORT_SSL  = 9443;

};

int main(int argc, const char** argv) {
    plog::ColorConsoleAppender<plog::TxtFormatterUtcTime> log_to_console;
    plog::RollingFileAppender <plog::TxtFormatterUtcTime> log_to_file(
        (boost::format{"%1%.log"} % argv[0]).str().c_str(), LOG_FILE_SIZE, LOG_FILE_NUMS);
    plog::init(plog::debug).addAppender(&log_to_console).addAppender(&log_to_file);
    const char* const db_user = std::getenv("POSTGRES_USER");
    const char* const db_pswd = std::getenv("POSTGRES_PASSWORD");
    const char* const db_name = std::getenv("POSTGRES_DB");
    const char* const db_salt = std::getenv("HASH_SALT");
    if (db_user == nullptr ||
            db_pswd == nullptr ||
                db_name == nullptr ||
                    db_salt == nullptr) {
        PLOG_FATAL << "bad database config";
        throw std::runtime_error{"bad database config"};
    }
    PLOG_INFO.printf(
        "starting service on %u (http) and %u (https) ports...", APP_PORT_HTTP, APP_PORT_SSL);
    Shortener::Application {
        Shortener::Application::cfg_db {
            .user = db_user,
            .pswd = db_pswd,
            .name = db_name,
            .salt = db_salt,
        },
        Shortener::Application::cfg_port {
            .http = APP_PORT_HTTP,
            .ssl  = APP_PORT_SSL ,
        },
    }.serve();
}
