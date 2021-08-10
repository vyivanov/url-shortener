#include "logger.h"
#include "config.h"
#include "application.h"
#include "database.h"
#include "postgres.h"
#include "xkcd/xkcdxx.h"

#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/http_defs.h>

#include <curl/curl.h>
#include <fmt/core.h>
#include <jinja2cpp/template.h>

#include <regex>
#include <iomanip>
#include <string>
#include <optional>
#include <cstdio>
#include <memory>
#include <filesystem>

namespace {

class CurlUnescape final {
public:
    explicit CurlUnescape(const std::string& url) noexcept
        : m_url(::curl_unescape(url.c_str(), url.length())) {
    }
    CurlUnescape(const CurlUnescape& other) = delete;
    CurlUnescape(CurlUnescape&& other) = delete;
    CurlUnescape& operator=(const CurlUnescape& other) = delete;
    CurlUnescape& operator=(CurlUnescape&& other) = delete;
   ~CurlUnescape() {
        ::curl_free((void*)m_url);
    }
    std::string operator()() const noexcept {
        return std::string{m_url};
    }
private:
    const char* const m_url;
};

constexpr const char* POSTGRES_CON_URI = R"(postgresql://{}:{}@storage/{})";
constexpr const char* REGEXP_VALID_URL = R"(^(http|https)://)";

};

namespace Shortener {

Application::Application(const cfg_db& db, const uint16_t port) noexcept
    : m_db(std::make_unique<Postgres>(
        fmt::format(POSTGRES_CON_URI, db.user, db.pswd, db.name),
        db.salt))
    , m_ep(std::make_unique<Pistache::Http::Endpoint>(
        Pistache::Address{Pistache::Ipv4::any(),
        Pistache::Port{port}})) {
    Pistache::Rest::Router routes = {};
    Pistache::Rest::Routes::Get(routes, "/"           , Pistache::Rest::Routes::bind(&Application::route_web    , this));
    Pistache::Rest::Routes::Get(routes, "/api"        , Pistache::Rest::Routes::bind(&Application::route_api    , this));
    Pistache::Rest::Routes::Get(routes, "/:key"       , Pistache::Rest::Routes::bind(&Application::route_key    , this));
    Pistache::Rest::Routes::Get(routes, "/favicon.ico", Pistache::Rest::Routes::bind(&Application::route_favicon, this));
    Pistache::Rest::Routes::Get(routes, "/api/ping"   , Pistache::Rest::Routes::bind(&Application::route_ping   , this));
    Pistache::Rest::Routes::NotFound(routes           , Pistache::Rest::Routes::bind(&Application::route_error  , this));
    m_ep->init(Pistache::Http::Endpoint::options().threads(std::thread::hardware_concurrency() << 1));
    m_ep->setHandler(routes.handler());
}

void Application::start() noexcept {
    m_ep->serveThreaded();
}

void Application::stop() noexcept {
    m_ep->shutdown();
}

void Application::route_web(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    log(request);
    try {
        if (const auto url = get_url(request); url.has_value()) {
            const auto key = m_db->insert(url.value(), get_host(request));
            const auto out = render_template("html/key.html.in", {{"root", APP_NAME}, {"key", key}});
            response.send(Pistache::Http::Code::Ok, out, MIME(Text, Html));
        } else {
            const auto [x, y, img, z, msg] = xkcdxx::Comic(xkcdxx::Comic::Number::Random).info();
            const auto out = render_template("html/web.html.in", {{"root", APP_NAME}, {"ver", APP_SEMVER}, {"img", img}, {"msg", msg}});
            response.send(Pistache::Http::Code::Ok, out, MIME(Text, Html));
        }
    } catch (const IDatabase::long_url& ex) {
        const auto out = render_template("html/err.html.in", {{"root", APP_NAME}, {"msg", "url is too long"}});
        response.send(Pistache::Http::Code::RequestURI_Too_Long, out, MIME(Text, Html));
    } catch (const std::exception& ex) {
        const auto out = render_template("html/err.html.in", {{"root", APP_NAME}, {"msg", ex.what()}});
        response.send(Pistache::Http::Code::Internal_Server_Error, out, MIME(Text, Html));
    }
}

void Application::route_api(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    log(request);
    try {
        if (const auto url = get_url(request); url.has_value()) {
            const auto key = m_db->insert(url.value(), get_host(request));
            const auto out = fmt::format("{}/{}", APP_NAME, key);
            response.send(Pistache::Http::Code::Ok, out, MIME(Text, Plain));
        } else {
            const auto out = render_template("html/api.html.in", {{"root", APP_NAME}});
            response.send(Pistache::Http::Code::Ok, out, MIME(Text, Html));
        }
    } catch (const IDatabase::long_url& ex) {
        response.send(Pistache::Http::Code::RequestURI_Too_Long, "url is too long", MIME(Text, Plain));
    } catch (const std::exception& ex) {
        response.send(Pistache::Http::Code::Internal_Server_Error, ex.what(), MIME(Text, Plain));
    }
}

void Application::route_key(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    log(request);
    try {
        const auto key = request.param(":key").as<std::string>();
        const auto url = m_db->search(key);
        const auto out = render_template("html/url.html.in", {{"root", APP_NAME}, {"url", url}});
        response.headers().add<Pistache::Http::Header::Location>(url);
        response.send(Pistache::Http::Code::Found, out, MIME(Text, Html));
    } catch (const IDatabase::undefined_key& ex) {
        const auto out = render_template("html/err.html.in", {{"root", APP_NAME}, {"msg", "resource not found"}});
        response.send(Pistache::Http::Code::Not_Found, out, MIME(Text, Html));
    } catch (const std::exception& ex) {
        const auto out = render_template("html/err.html.in", {{"root", APP_NAME}, {"msg", ex.what()}});
        response.send(Pistache::Http::Code::Internal_Server_Error, out, MIME(Text, Html));
    }
}

void Application::route_favicon(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    log(request);
    response.headers().add<Pistache::Http::Header::ContentType>("image/x-icon");
    Pistache::Http::serveFile(response, "favicon.ico");
}

void Application::route_ping(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    log(request);
    if (m_db->ping()) {
        response.send(Pistache::Http::Code::Ok, "I'm alive!", MIME(Text, Plain));
    } else {
        response.send(Pistache::Http::Code::Internal_Server_Error, "Bad news in logs...", MIME(Text, Plain));
    }
}

void Application::route_error(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    log(request);
    const auto out = render_template("html/err.html.in", {{"root", APP_NAME}, {"msg", "resource not found"}});
    response.send(Pistache::Http::Code::Not_Found, out, MIME(Text, Html));
}

void Application::log(const Pistache::Rest::Request& request) noexcept {
    LOG_INFO("GET {}{} from {}", request.resource(), request.query().as_str(), get_host(request));
}

std::optional<std::string> Application::get_url(const Pistache::Rest::Request& request) noexcept {
    if (request.query().get("url") &&
            request.query().get("url").value().empty() == false) {
        const std::string inp_url = request.query().get("url").value();
        const std::string out_url = CurlUnescape{inp_url}();
        if (std::regex_search(out_url, std::regex{REGEXP_VALID_URL})) {
            return out_url;
        } else {
            return fmt::format("http://{}", out_url);
        }
    }
    return std::nullopt;
}

std::string Application::get_host(const Pistache::Rest::Request& request) noexcept {
    const std::optional<Pistache::Http::Header::Raw> host = request.headers().tryGetRaw("X-Real-IP");
    if (host.has_value()) {
        return host.value().value();
    }
    return request.address().host();
}

std::string Application::render_template(const std::filesystem::path& file, const jinja2::ValuesMap& attr) noexcept {
    auto tmpl = jinja2::Template{};
    tmpl.LoadFromFile(file);
    return tmpl.RenderAsString(attr).value();
}

};
