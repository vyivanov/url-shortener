#include "config.h"
#include "application.h"

#include <jinja2cpp/template.h>

#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/http_defs.h>

#include <curl/curl.h>
#include <boost/format.hpp>

#include <mutex>
#include <regex>
#include <iomanip>
#include <string>
#include <string_view>
#include <optional>
#include <cstdio>

namespace {

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

class CurlRAII final {
public:
    CurlRAII() noexcept : m_curl{curl_easy_init()} {
        ;;;
    }
    CurlRAII(const CurlRAII& rh) = delete;
    CurlRAII(CurlRAII&& rh) = delete;
    CurlRAII& operator=(const CurlRAII& rh) = delete;
    CurlRAII& operator=(CurlRAII&& rh) = delete;
   ~CurlRAII() {
        curl_easy_cleanup(m_curl);
    }
    CURL* handle() const noexcept {
        return m_curl;
    }
private:
    CURL* const m_curl;
};

constexpr const char* POSTGRES_CON_URI = R"(postgresql://%1%:%2%@postgres/%3%)";
constexpr const char* REGEXP_VALID_URL = R"(^(http|https|ftp)://)";

};

namespace Shortener {

Application::Application(const cfg_db& db, const uint16_t port) noexcept
: m_db{(boost::format{POSTGRES_CON_URI} %
    db.user %
    db.pswd %
    db.name).str(),
    db.salt}
, m_port{Pistache::Port{port}} {
    Get(m_router, "/"           , bind(&Application::request_web, this));
    Get(m_router, "/api"        , bind(&Application::request_api, this));
    Get(m_router, "/:key"       , bind(&Application::request_key, this));
    Get(m_router, "/favicon.ico", bind(&Application::request_ico, this));
    NotFound(m_router           , bind(&Application::request_err, this));
}

void Application::serve() noexcept {
    const auto addr = Pistache::Address{Pistache::Ipv4::any(), m_port};
    const auto opts = Endpoint::options().threads(std::thread::hardware_concurrency());
    Endpoint service{addr};
    service.init(opts);
    service.setHandler(m_router.handler());
    service.serve();
}

void Application::request_web(const Request& request, ResponseWriter response) {
    log(request);
    if (const auto url = get_url(request); url) {
        try {
            const auto key = m_db.insert(url.value(), request.address().host());
            const auto out = render_template("html/key.html.in", {{"root", APP_NAME}, {"key", key}});
            response.send(Code::Ok, out, MIME(Text, Html));
        } catch (const Database::long_url&) {
            const auto out = render_template("html/web.html.in", {{"root", APP_NAME}});
            response.send(Code::RequestURI_Too_Long, out, MIME(Text, Html));
        }
    } else {
        const auto out = render_template("html/web.html.in", {{"root", APP_NAME}});
        response.send(Code::Ok, out, MIME(Text, Html));
    }
}

void Application::request_api(const Request& request, ResponseWriter response) {
    log(request);
    if (const auto url = get_url(request); url) {
        try {
            const auto key = m_db.insert(url.value(), request.address().host());
            const auto out = boost::format{"http://%1%/%2%"} % APP_NAME % key;
            response.send(Code::Ok, out.str(), MIME(Text, Plain));
        } catch (Database::long_url&) {
            response.send(Code::RequestURI_Too_Long, "url is too long", MIME(Text, Plain));
        }
    } else {
        const auto out = render_template("html/api.html.in", {{"root", APP_NAME}});
        response.send(Code::Ok, out, MIME(Text, Html));
    }
}

void Application::request_key(const Request& request, ResponseWriter response) {
    log(request);
    try {
        const auto key = request.param(":key").as<std::string>();
        const auto url = m_db.search(key);
        const auto out = render_template("html/url.html.in", {{"root", APP_NAME}, {"url", url}});
        response.headers().add<Location>(url);
        response.send(Code::Found, out, MIME(Text, Html));
    } catch (const Database::undefined_key&) {
        const auto out = render_template("html/err.html.in", {{"root", APP_NAME}});
        response.send(Code::Not_Found, out, MIME(Text, Html));
    }
}

void Application::request_ico(const Request& request, ResponseWriter response) {
    log(request);
    response.headers().add<ContentType>("image/x-icon");
    serveFile(response, "favicon.ico");
}

void Application::request_err(const Request& request, ResponseWriter response) {
    log(request);
    const auto out = render_template("html/err.html.in", {{"root", APP_NAME}});
    response.send(Code::Not_Found, out, MIME(Text, Html));
}

void Application::log(const Request& request) const noexcept {
    const auto ts = std::time(nullptr);
    const auto lk = std::lock_guard{m_mtx};
    std::stringstream timestamp{};
    timestamp << std::put_time(std::gmtime(&ts), "%F %T %Z");
    std::fprintf(::stdout, "[%s] %s %s %s (%s)\n"           ,
                            timestamp.str().c_str()         ,
                            methodString(request.method())  ,
                            request.resource().c_str()      ,
                            request.query().as_str().c_str(),
                            request.address().host().c_str());
    std::fflush(::stdout);
}

std::optional<std::string> Application::get_url(const Request& request) noexcept {
    if (request.query().has("url") &&
            request.query().get("url").get().empty() == false) {
        const std::string inp_url = request.query().get("url").get();
        const std::string out_url = curl_easy_unescape(CurlRAII{}.handle(), inp_url.c_str(), 0, nullptr);
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
