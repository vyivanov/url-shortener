#pragma once

#include "database.h"

#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <jinja2cpp/template.h>

#include <string>
#include <optional>
#include <filesystem>
#include <memory>

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
    void start() noexcept;
    void stop () noexcept;
private:
    void route_web    (const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void route_api    (const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void route_key    (const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void route_favicon(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void route_humans (const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void route_ping   (const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void route_error  (const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
private:
    static void log(const Pistache::Rest::Request& request) noexcept;
    static std::optional<std::string> get_url(const Pistache::Rest::Request& request) noexcept;
    static std::string get_host(const Pistache::Rest::Request& request) noexcept;
    static std::string render_template(const std::filesystem::path& file, const jinja2::ValuesMap& attr) noexcept;
private:
    const std::unique_ptr<IDatabase>                m_db;
    const std::unique_ptr<Pistache::Http::Endpoint> m_ep;
};

}
