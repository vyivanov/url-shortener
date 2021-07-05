#include "application.h"

#include <cstdlib>
#include <exception>

int main() {
    const char* const db_name = std::getenv("POSTGRES_DB");
    const char* const db_user = std::getenv("POSTGRES_USER");
    const char* const db_pswd = std::getenv("POSTGRES_PASSWORD");
    if (db_name == nullptr ||
            db_user == nullptr ||
                db_pswd == nullptr) {
        throw std::runtime_error{"bad database config"};
    }
    const auto db = Shortener::Application::cfg_db{
        .name = db_name,
        .user = db_user,
        .pswd = db_pswd,
    };
    Shortener::Application{db, 9080}.serve();
}
