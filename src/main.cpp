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
        constexpr const char* fatal_msg = "bad database config";
        PLOG_FATAL << fatal_msg;
        throw std::runtime_error{fatal_msg};
    }
    const auto cfg_db = Shortener::Application::cfg_db{
        .user = db_user,
        .pswd = db_pswd,
        .name = db_name,
        .salt = db_salt,
    };
    PLOG_INFO << "starting application service...";
    Shortener::Application{cfg_db, 80}.serve();
}
