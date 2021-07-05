#pragma once

#include "database.h"

#include <pistache/router.h>
#include <jinja2cpp/template.h>

#include <mutex>
#include <string>
#include <string_view>
#include <optional>

namespace Shortener {

using Pistache::Rest::Request;
using Pistache::Http::ResponseWriter;

class Application final {
public:
    struct cfg_db {
        std::string name;
        std::string user;
        std::string pswd;
    };
    Application(const cfg_db& db, uint16_t port) noexcept;
    Application(const Application& rh) = delete;
    Application(Application&& rh) = delete;
    Application& operator=(const Application& rh) = delete;
    Application& operator=(Application&& rh) = delete;
    [[noreturn]] void serve() noexcept;
private:
    void request_web(const Request& request, ResponseWriter response);
    void request_api(const Request& request, ResponseWriter response);
    void request_key(const Request& request, ResponseWriter response);
    void request_ico(const Request& request, ResponseWriter response);
    void request_err(const Request& request, ResponseWriter response);
private:
    void log(const Request& request) const noexcept;
    static std::optional<std::string> get_url(const Request& request) noexcept;
    static std::string render_template(const std::string& file, const jinja2::ValuesMap& attr) noexcept;
private:
    std::mutex mutable     m_mtx;
    Pistache::Rest::Router m_router;
    Database               m_db;
    const Pistache::Port   m_port;
};

}
