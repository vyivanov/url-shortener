#include "application.h"

#include <cstdlib>
#include <exception>

int main() {
    const char* const db_user = std::getenv("POSTGRES_USER");
    const char* const db_pswd = std::getenv("POSTGRES_PASSWORD");
    const char* const db_name = std::getenv("POSTGRES_DB");
    const char* const db_salt = std::getenv("HASH_SALT");
    if (db_user == nullptr ||
            db_pswd == nullptr ||
                db_name == nullptr ||
                    db_salt == nullptr) {
        throw std::runtime_error{"bad database config"};
    }
    const auto cfg_db = Shortener::Application::cfg_db{
        .user = db_user,
        .pswd = db_pswd,
        .name = db_name,
        .salt = db_salt,
    };
    Shortener::Application{cfg_db, 9080}.serve();
}
