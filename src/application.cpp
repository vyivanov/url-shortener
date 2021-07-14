#include "config.h"
#include "application.h"
#include "database.h"
#include "postgres.h"
#include "xkcd.h"

#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/http_defs.h>

#include <plog/Log.h>
#include <curl/curl.h>
#include <boost/format.hpp>
#include <jinja2cpp/template.h>

#include <regex>
#include <iomanip>
#include <string>
#include <optional>
#include <cstdio>
#include <memory>

namespace {

class CurlUnescape final {
public:
    explicit CurlUnescape(const std::string& url) noexcept
        : m_url(::curl_easy_unescape(nullptr, url.c_str(), 0, nullptr)) {
    }
    CurlUnescape(const CurlUnescape& rh) = delete;
    CurlUnescape(CurlUnescape&& rh) = delete;
    CurlUnescape& operator=(const CurlUnescape& rh) = delete;
    CurlUnescape& operator=(CurlUnescape&& rh) = delete;
   ~CurlUnescape() {
        ::curl_free((void*)m_url);
    }
    std::string operator()() const noexcept {
        return std::string{m_url};
    }
private:
    const char* const m_url;
};

using Pistache::Rest::Request;
using Pistache::Rest::Routes::Get;
using Pistache::Rest::Routes::NotFound;
using Pistache::Rest::Routes::bind;
using Pistache::Http::Code;
using Pistache::Http::Endpoint;
using Pistache::Http::serveFile;
using Pistache::Http::ResponseWriter;
using Pistache::Http::methodString;
using Pistache::Http::Header::Location;
using Pistache::Http::Header::ContentType;

constexpr const char* POSTGRES_CON_URI = R"(postgresql://%1%:%2%@postgres/%3%)";
constexpr const char* REGEXP_VALID_URL = R"(^(http|https)://)";

};

#define ROUTE_LOG(request) \
    PLOG_INFO << request.method()         << '\x20' \
              << request.resource()       << '\x20' \
              << request.query().as_str() << '\x20' \
              << request.address().host()

namespace Shortener {

Application::Application(const cfg_db& db, const uint16_t port) noexcept
: m_db(std::make_unique<Postgres>(
    (boost::format{POSTGRES_CON_URI} %
        db.user %
        db.pswd %
        db.name).str(),
    db.salt))
, m_port(Pistache::Port{port}) {
    Get(m_router, "/"           , bind(&Application::request_web, this));
    Get(m_router, "/api"        , bind(&Application::request_api, this));
    Get(m_router, "/:key"       , bind(&Application::request_key, this));
    Get(m_router, "/favicon.ico", bind(&Application::request_ico, this));
    Get(m_router, "/api/check"  , bind(&Application::request_chk, this));
    NotFound(m_router           , bind(&Application::request_err, this));
}

void Application::serve() noexcept {
    const auto addr = Pistache::Address{Pistache::Ipv4::any(), m_port};
    const auto opts = Endpoint::options().threads(std::thread::hardware_concurrency() << 1);
    Endpoint service{addr};
    service.init(opts);
    service.setHandler(m_router.handler());
    service.serve();
}

void Application::request_web(const Request& request, ResponseWriter response) {
    ROUTE_LOG(request);
    if (const auto url = get_url(request); url.has_value()) {
        try {
            const auto key = m_db->insert(url.value(), request.address().host());
            const auto out = render_template("html/key.html.in", {{"root", APP_NAME}, {"key", key}});
            response.send(Code::Ok, out, MIME(Text, Html));
        } catch (const IDatabase::long_url&) {
            const auto out = render_template("html/err.html.in", {{"root", APP_NAME}, {"msg", "url is too long"}});
            response.send(Code::RequestURI_Too_Long, out, MIME(Text, Html));
        }
    } else {
        const auto [img, msg] = xkcdxx::get_random_comic();
        const auto out = render_template("html/web.html.in", {{"root", APP_NAME}, {"ver", APP_SEMVER}, {"img", img}, {"msg", msg}});
        response.send(Code::Ok, out, MIME(Text, Html));
    }
}

void Application::request_api(const Request& request, ResponseWriter response) {
    ROUTE_LOG(request);
    if (const auto url = get_url(request); url.has_value()) {
        try {
            const auto key = m_db->insert(url.value(), request.address().host());
            const auto out = boost::format{"http://%1%/%2%"} % APP_NAME % key;
            response.send(Code::Ok, out.str(), MIME(Text, Plain));
        } catch (const IDatabase::long_url&) {
            response.send(Code::RequestURI_Too_Long, "url is too long", MIME(Text, Plain));
        }
    } else {
        const auto out = render_template("html/api.html.in", {{"root", APP_NAME}});
        response.send(Code::Ok, out, MIME(Text, Html));
    }
}

void Application::request_key(const Request& request, ResponseWriter response) {
    ROUTE_LOG(request);
    try {
        const auto key = request.param(":key").as<std::string>();
        const auto url = m_db->search(key);
        const auto out = render_template("html/url.html.in", {{"root", APP_NAME}, {"url", url}});
        response.headers().add<Location>(url);
        response.send(Code::Found, out, MIME(Text, Html));
    } catch (const IDatabase::undefined_key&) {
        const auto out = render_template("html/err.html.in", {{"root", APP_NAME}, {"msg", "resource not found"}});
        response.send(Code::Not_Found, out, MIME(Text, Html));
    }
}

void Application::request_ico(const Request& request, ResponseWriter response) {
    ROUTE_LOG(request);
    response.headers().add<ContentType>("image/x-icon");
    serveFile(response, "favicon.ico");
}

void Application::request_chk(const Request& request, ResponseWriter response) {
    ROUTE_LOG(request);
    if (m_db->check()) {
        response.send(Code::Ok, "I'm alive!", MIME(Text, Plain));
    } else {
        response.send(Code::Internal_Server_Error, "Bad news...", MIME(Text, Plain));
    }
}

void Application::request_err(const Request& request, ResponseWriter response) {
    ROUTE_LOG(request);
    const auto out = render_template("html/err.html.in", {{"root", APP_NAME}, {"msg", "resource not found"}});
    response.send(Code::Not_Found, out, MIME(Text, Html));
}

std::optional<std::string> Application::get_url(const Request& request) noexcept {
    if (request.query().has("url") &&
            request.query().get("url").get().empty() == false) {
        const std::string inp_url = request.query().get("url").get();
        const std::string out_url = CurlUnescape{inp_url}();
        if (std::regex_search(out_url, std::regex{REGEXP_VALID_URL})) {
            return out_url;
        } else {
            return std::string{"http://"} + out_url;
        }
    }
    return std::nullopt;
}

std::string Application::render_template(const std::string& file, const jinja2::ValuesMap& attr) noexcept {
    auto tmpl = jinja2::Template{};
    tmpl.LoadFromFile(file);
    return tmpl.RenderAsString(attr).value();
}

};
