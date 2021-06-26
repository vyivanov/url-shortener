#pragma once

#include <pistache/router.h>

namespace Shortener {

using Pistache::Rest::Request;
using Pistache::Http::ResponseWriter;

class Application final {

public:

    explicit Application(uint16_t port);
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

    static void log(const Request& request);
    static std::optional<std::string> get_url(const Request& request);

private:

    const Pistache::Port   m_port;
    Pistache::Rest::Router m_router;
};

}
