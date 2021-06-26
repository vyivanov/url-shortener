#include "application.h"

#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/http_defs.h>

#include <iomanip>

namespace Shortener {

using Pistache::Rest::Routes::Get;
using Pistache::Rest::Routes::NotFound;
using Pistache::Rest::Routes::bind;
using Pistache::Http::Endpoint;
using Pistache::Rest::Request;
using Pistache::Http::ResponseWriter;
using Pistache::Http::Code;
using Pistache::Http::serveFile;

Application::Application(const uint16_t port) : m_port(Pistache::Port(port))
{
    Get(m_router, "/"    , bind(&Application::req_web_short, this));
    Get(m_router, "/api" , bind(&Application::req_api_short, this));
    Get(m_router, "/:key", bind(&Application::req_redirect , this));
    NotFound(m_router    , bind(&Application::req_invalid  , this));
}

void Application::serve()
{
    auto addr = Pistache::Address(Pistache::Ipv4::any(), m_port);
    auto opts = Endpoint::options().threads(std::thread::hardware_concurrency());

    Endpoint service(addr);

    service.init(opts);
    service.setHandler(m_router.handler());
    service.serve();
}

void Application::req_web_short(const Request& request, ResponseWriter response)
{
    this->log(request);

    std::optional<std::string> url = request.query().get("url");

    if (url) {
        response.send(Code::Ok, "<center><a href=\"#\">http://clck.app/q2Y</a></center>", MIME(Text, Html));
    } else {
        serveFile(response, "index.html", MIME(Text, Html));
    }
}

void Application::req_api_short(const Request& request, ResponseWriter response)
{
    this->log(request);

    std::optional<std::string> url = request.query().get("url");

    response.send(Code::Ok, std::string{"req_api_short "} + url.value_or(""));
}

void Application::req_redirect(const Request& request, ResponseWriter response)
{
    this->log(request);

    auto id = request.param(":key").as<std::string>();
    response.send(Code::Ok, id);
}

void Application::req_invalid(const Request& request, ResponseWriter response)
{
    this->log(request);

    response.send(Code::Not_Found, "req_invalid");
}

void Application::log(const Request& request)
{
    const auto ts = std::time(nullptr);

    std::clog
        << "["
        << std::put_time(std::gmtime(&ts), "%F %T %Z")
        << "]""\x20"
        << request.method()         << "\x20"
        << request.resource()       << "\x20"
        << request.query().as_str() << "\x20"
        << "\n";
}

}
