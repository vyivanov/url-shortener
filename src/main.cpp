#include "logger.h"
#include "application.h"

#include <signal.h>
#include <string.h>

#include <string>
#include <cstdlib>
#include <cerrno>

namespace {

class Signals final {
public:
    Signals() noexcept {
        m_signum          = 0;
        m_sigact.sa_flags = 0;
        ::sigemptyset(&m_sigact.sa_mask);
        ::sigaddset(&m_sigact.sa_mask , SIGINT );
        ::sigaddset(&m_sigact.sa_mask , SIGQUIT);
        ::sigaddset(&m_sigact.sa_mask , SIGTERM);
        ::sigaction(SIGINT , &m_sigact, nullptr);
        ::sigaction(SIGQUIT, &m_sigact, nullptr);
        ::sigaction(SIGTERM, &m_sigact, nullptr);
    }
    std::string wait() noexcept {
        ::sigwait(&m_sigact.sa_mask, &m_signum);
        return ::strsignal(m_signum);
    }
private:
    using sigact_t = struct sigaction;
    int      m_signum;
    sigact_t m_sigact;
};

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
    Shortener::Application app {
        Shortener::Application::cfg_db {
            .user = db_user,
            .pswd = db_pswd,
            .name = db_name,
            .salt = db_salt,
        },
        APP_PORT
    };
    app.start();
    LOG_INFO("service started on {} (http) port", APP_PORT);
    const auto sig = Signals{}.wait();
    app.stop();
    LOG_INFO("service gracefully stopped due to '{}' signal", sig);
}
