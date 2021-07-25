#include "client.h"
#include "xkcdxx.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#define PICOJSON_USE_INT64
#include "picojson.h"

#include <string>
#include <ctime>
#include <cstdint>

namespace xkcdxx::client {

Xkcd::Xkcd() noexcept: m_xkcd("https://xkcd.com") {
    m_xkcd.set_keep_alive(true);
    m_xkcd.set_follow_location(true);
    m_xkcd.enable_server_certificate_verification(false);
}

Comic::Info Xkcd::get_info(const uint64_t num) {
    const std::string path = (num > 0) ?
        std::string("/") + std::to_string(num) + std::string("/info.0.json") : std::string("/info.0.json");
    const httplib::Result response = m_xkcd.Get(path.c_str());
    if (response && response->status == 200) {
        picojson::value json = {};
        picojson::parse(json, response->body);
        std::tm date = {
            .tm_mday = std::stoi(json.get("day")  .to_str()) -    0,
            .tm_mon  = std::stoi(json.get("month").to_str()) -    1,
            .tm_year = std::stoi(json.get("year") .to_str()) - 1900,
        };
        return Comic::Info {
            .number = json.get("num")  .get<int64_t>(),
            .date   = std::mktime(&date)              ,
            .url    = json.get("img")  .to_str()      ,
            .title  = json.get("title").to_str()      ,
            .alt    = json.get("alt")  .to_str()      ,
        };
    } else {
        if (response) {
            const auto why = std::string("xkcd.com returned\x20") +
                             std::to_string(response->status)     +
                             std::string("\x20status code");
            throw Comic::request_failed(why);
        } else {
            const auto why = std::string("network error\x20") +
                             std::to_string(static_cast<int>(response.error()));
            throw Comic::request_failed(why);
        }
    }
}

};
