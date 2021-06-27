#pragma once

#include <pistache/router.h>
#include <jinja2cpp/template.h>

#include <mutex>
#include <string>
#include <optional>

namespace Shortener {

using Pistache::Rest::Request;
using Pistache::Http::ResponseWriter;

class Application final {
public:
    Application();
    Application(const Application& rh) = delete;
    Application(Application&& rh) = delete;
   ~Application() = default;
    Application& operator=(const Application& rh) = delete;
    Application& operator=(Application&& rh) = delete;
    [[noreturn]] void serve();
private:
    void request_web(const Request& request, ResponseWriter response);
    void request_api(const Request& request, ResponseWriter response);
    void request_key(const Request& request, ResponseWriter response);
    void request_err(const Request& request, ResponseWriter response);
    void log(const Request& request);
private:
    static std::optional<std::string> get_url(const Request& request);
    static std::string render_template(const std::string& file, const jinja2::ValuesMap& attr);
private:
    std::mutex             m_mtx;
    Pistache::Rest::Router m_router;
};

}
