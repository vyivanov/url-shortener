#include <pqxx/pqxx>

#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <iostream>

class Application {
public:
    Application() {
        Pistache::Rest::Routes::Get(m_router, "/",     Pistache::Rest::Routes::bind(&Application::shortening_via_web, this));
        Pistache::Rest::Routes::Get(m_router, "/api",  Pistache::Rest::Routes::bind(&Application::shortening_via_api, this));
        Pistache::Rest::Routes::Get(m_router, "/:key", Pistache::Rest::Routes::bind(&Application::redirecting, this));

        Pistache::Rest::Routes::NotFound(m_router, Pistache::Rest::Routes::bind(&Application::not_found, this));
    }

    std::shared_ptr<Pistache::Http::Handler> operator()() {
        return m_router.handler();
    }

    void shortening_via_web(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
        std::clog << "-->\n";
        std::optional<std::string> url = request.query().get("url");
        response.send(Pistache::Http::Code::Ok, std::string{"<b>shortening_via_web</b> "} + url.value_or(""));
    }

    void shortening_via_api(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
        std::optional<std::string> url = request.query().get("url");
        response.send(Pistache::Http::Code::Ok, std::string{"shortening_via_api "} + url.value_or(""));
    }

    void redirecting(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
        auto id = request.param(":key").as<std::string>();
        response.send(Pistache::Http::Code::Ok, id);
    }

    void not_found(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
        response.send(Pistache::Http::Code::Ok, "kek");
    }

private:
    Pistache::Rest::Router m_router;
};

int main() {
    pqxx::connection db{"postgresql://url:test@localhost/url"};
    pqxx::work wrk{db};

    pqxx::result res{wrk.exec("SELECT * FROM url")};
    wrk.commit();
    for (const auto row: res) {
        std::cout
            << row["idx"].c_str() << '\x20'
            << row["url"].c_str() << '\x20'
            << row["tms"].c_str() << '\x20'
            << row["ipc"].c_str() << '\x20'
            << row["cnt"].c_str() << '\x20'
            << '\n';
    }

    auto logger = PISTACHE_DEFAULT_STRING_LOGGER;

    Application app;

    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(9080));
    auto opts = Pistache::Http::Endpoint::options().threads(std::thread::hardware_concurrency());

    Pistache::Http::Endpoint service(addr);
    service.init(opts);
    service.setHandler(app());
    service.serve();
}
