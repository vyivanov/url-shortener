#include "logger.h"
#include "application.h"

#include <cstdlib>
#include <cerrno>

namespace {

constexpr uint16_t APP_PORT = 9080;

};

int main() {
    const char* const db_user = std::getenv("POSTGRES_USER");
    const char* const db_pswd = std::getenv("POSTGRES_PASSWORD");
    const char* const db_name = std::getenv("POSTGRES_DB");
    const char* const db_salt = std::getenv("HASH_SALT");
    if (db_user == nullptr ||
            db_pswd == nullptr ||
                db_name == nullptr ||
                    db_salt == nullptr) {
        LOG_CRITICAL("bad database config");
        return -EINVAL;
    }
    LOG_INFO("starting service on {} (http) port...", APP_PORT);
    Shortener::Application {
        Shortener::Application::cfg_db {
            .user = db_user,
            .pswd = db_pswd,
            .name = db_name,
            .salt = db_salt,
        },
        APP_PORT
    }.serve();
}
