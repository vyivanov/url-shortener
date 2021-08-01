#pragma once

#include "database.h"

#include <pistache/router.h>
#include <jinja2cpp/template.h>

#include <string>
#include <optional>
#include <memory>

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
    Application(const Application& other) = delete;
    Application(Application&& other) = delete;
    Application& operator=(const Application& other) = delete;
    Application& operator=(Application&& other) = delete;
    void serve() noexcept;
private:
    void request_web(const Request& request, ResponseWriter response);
    void request_api(const Request& request, ResponseWriter response);
    void request_key(const Request& request, ResponseWriter response);
    void request_favicon(const Request& request, ResponseWriter response);
    void request_ping(const Request& request, ResponseWriter response);
    void request_error(const Request& request, ResponseWriter response);
private:
    static std::optional<std::string> get_url(const Request& request) noexcept;
    static std::string get_host(const Request& request) noexcept;
    static std::string render_template(const std::string& file, const jinja2::ValuesMap& attr) noexcept;
private:
    Pistache::Rest::Router           m_router;
    const std::unique_ptr<IDatabase> m_db;
    const Pistache::Port             m_port;
};

};
