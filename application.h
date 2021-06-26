#pragma once

#include <pistache/router.h>

namespace Shortener {

using Pistache::Rest::Request;
using Pistache::Http::ResponseWriter;

class Application final {
public:
    explicit Application(const uint16_t port);
   ~Application() = default;
    Application(const Application& rh) = delete;
    Application(Application&& rh) = delete;
    Application& operator=(const Application& rh) = delete;
    Application& operator=(Application&& rh) = delete;
    [[noreturn]] void serve();
private:
    void req_web_short(const Request& request, ResponseWriter response);
    void req_api_short(const Request& request, ResponseWriter response);
    void req_redirect (const Request& request, ResponseWriter response);
    void req_invalid  (const Request& request, ResponseWriter response);
    static void log   (const Request& request);
    const Pistache::Port m_port;
    Pistache::Rest::Router m_router;
};

}
