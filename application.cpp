#include "application.h"

#include <jinja2cpp/template.h>

#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/http_defs.h>

#include <boost/format.hpp>

#include <mutex>
#include <iomanip>
#include <optional>

namespace Shortener {

using Pistache::Rest::Request;
using Pistache::Rest::Routes::Get;
using Pistache::Rest::Routes::NotFound;
using Pistache::Rest::Routes::bind;
using Pistache::Http::Code;
using Pistache::Http::Endpoint;
using Pistache::Http::serveFile;
using Pistache::Http::ResponseWriter;

Application::Application() {
    Get(m_router, "/"    , bind(&Application::request_web, this));
    Get(m_router, "/api" , bind(&Application::request_api, this));
    Get(m_router, "/:key", bind(&Application::request_key, this));
    NotFound(m_router    , bind(&Application::request_err, this));
}

void Application::serve() {
    auto addr = Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(9080));
    auto opts = Endpoint::options().threads(std::thread::hardware_concurrency());
    Endpoint service(addr);
    service.init(opts);
    service.setHandler(m_router.handler());
    service.serve();
}

void Application::request_web(const Request& request, ResponseWriter response) {
    log(request);
    if (auto url = get_url(request); url) {
        auto out = render_template("html/key.html.in", {{"key", url.value()}});     // TODO: url -> key
        response.send(Code::Ok, out.c_str(), MIME(Text, Html));
    } else {
        serveFile(response, "html/web.html", MIME(Text, Html));
    }
}

void Application::request_api(const Request& request, ResponseWriter response) {
    log(request);
    if (auto url = get_url(request); url) {
        auto out = boost::format("http://clck.app/%1%") % url.value();              // TODO: url -> key
        response.send(Code::Ok, out.str(), MIME(Text, Plain));
    } else {
        serveFile(response, "html/api.html", MIME(Text, Html));
    }
}

void Application::request_key(const Request& request, ResponseWriter response) {
    log(request);
    auto key = request.param(":key").as<std::string>();
    auto out = render_template("html/url.html.in", {{"url", key.c_str()}});         // TODO: key -> url
    response.send(Code::Ok, out.c_str(), MIME(Text, Html));
}

void Application::request_err(const Request& request, ResponseWriter response) {
    log(request);
    serveFile(response, "html/err.html", MIME(Text, Html));
}

void Application::log(const Request& request) {
    const auto ts = std::time(nullptr);
    const auto lk = std::lock_guard(m_mtx);
    std::clog
        << "["
        << std::put_time(std::gmtime(&ts), "%F %T %Z")
        << "]""\x20"
        << request.method()         << "\x20"
        << request.resource()       << "\x20"
        << request.query().as_str() << "\x20"
        << "\n";
}

std::optional<std::string> Application::get_url(const Request& request) {
    if (request.query().has("url") &&
            request.query().get("url").get().empty() == false) {
        return request.query().get("url").get();
    }
    return std::nullopt;
}

std::string Application::render_template(const std::string& file, const jinja2::ValuesMap& attr) {
    auto temp = jinja2::Template();
    temp.LoadFromFile(file);
    return temp.RenderAsString(attr).value();
}

}
