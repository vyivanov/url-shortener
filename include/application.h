#pragma once

#include "postgres.h"

#include <pistache/router.h>
#include <jinja2cpp/template.h>

#include <string>
#include <optional>

namespace {

using Pistache::Rest::Request;
using Pistache::Http::ResponseWriter;

};

namespace Shortener {

class Application final {
public:
    struct cfg_db {
        std::string user;
        std::string pswd;
        std::string name;
        std::string salt;
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
    void request_chk(const Request& request, ResponseWriter response);
    void request_err(const Request& request, ResponseWriter response);
private:
    static std::optional<std::string> get_url(const Request& request) noexcept;
    static std::string render_template(const std::string& file, const jinja2::ValuesMap& attr) noexcept;
private:
    Pistache::Rest::Router m_router;
    Postgres               m_db;
    Pistache::Port const   m_port;
};

};
