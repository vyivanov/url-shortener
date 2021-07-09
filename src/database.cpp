#include "database.h"

#include <pqxx/pqxx>
#include <boost/format.hpp>

#include <string>
#include <vector>

namespace {

constexpr const char* TMPL_INSERT_ITEM = "INSERT INTO public.item (url, ipc) VALUES('%1%', '%2%') RETURNING idx";
constexpr const char* TMPL_SEARCH_ITEM = "UPDATE public.item SET cnt = cnt + 1 WHERE idx = %1%;"
                                         "SELECT url FROM public.item WHERE idx = %1%";

constexpr uint32_t MAX_URL_LENGTH = 1'024UL;

};

namespace Shortener {

Database::Database(const std::string& uri, const std::string& salt) noexcept : m_uri{uri}, m_key{salt, 3} {
    ;;;
}

std::string Database::insert(const std::string& url, const std::string& ipc) {
    if (url.length() > MAX_URL_LENGTH) {
        throw long_url{"url is too long"};
    }
    const auto out = do_request((boost::format{TMPL_INSERT_ITEM} % url % ipc).str());
    const auto idx = out.at(0).at("idx").c_str();
    return m_key.encode(std::stoi(idx));
}

std::string Database::search(const std::string& key) {
    const auto idx = [&]() -> uint64_t {
        const std::vector<uint64_t> idx = m_key.decode(key);
        if (idx.empty()) {
            throw undefined_key{"key not found"};
        } else {
            return idx.at(0);
        }
    }();
    const auto out = do_request((boost::format{TMPL_SEARCH_ITEM} % idx).str());
    if (out.empty()) {
        throw undefined_key{"key not found"};
    }
    return out.at(0).at("url").c_str();
}

pqxx::result Database::do_request(const std::string& sql) noexcept {
    pqxx::connection database{m_uri};
    pqxx::work request{database};
    pqxx::result result{request.exec(sql)};
    request.commit();
    return result;
}

};
