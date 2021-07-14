#include "postgres.h"

#include <pqxx/pqxx>
#include <boost/format.hpp>

#include <string>
#include <vector>
#include <mutex>
#include <optional>

namespace {

constexpr const char* TMPL_SEARCH_URL = "SELECT idx FROM public.item WHERE url = '%1%' LIMIT 1";
constexpr const char* TMPL_INSERT_URL = "INSERT INTO public.item (url, ipc) VALUES('%1%', '%2%') RETURNING idx";
constexpr const char* TMPL_SEARCH_IDX = "SELECT url FROM public.item WHERE idx = %1%";
constexpr const char* TMPL_UPDATE_CNT = "UPDATE public.item SET cnt = cnt + 1 WHERE idx = %1%";

constexpr uint32_t MAX_URL_LENGTH = 1'024UL;

};

namespace Shortener {

Database::Database(const std::string& uri, const std::string& salt) noexcept
    : m_uri{uri}
    , m_key{salt, 3} {
}

std::string Database::insert(const std::string& url, const std::string& ipc) {
    if (url.length() > MAX_URL_LENGTH) {
        throw long_url{"url is too long"};
    }
    const auto idx = [&]() -> std::string {
        std::lock_guard lg{m_mtx};
        const auto search_url = boost::format{TMPL_SEARCH_URL} % url;
        if (const auto out = do_request(search_url.str()); out.has_value()) {
            return out.value().at("idx").c_str();
        }
        const auto insert_url = boost::format{TMPL_INSERT_URL} % url % ipc;
        const auto out = do_request(insert_url.str());
        return out.value().at("idx").c_str();
    }();
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
    std::lock_guard lg{m_mtx};
    const auto search_idx = boost::format{TMPL_SEARCH_IDX} % idx;
    if (const auto out = do_request(search_idx.str()); out.has_value()) {
        const auto update_cnt = boost::format{TMPL_UPDATE_CNT} % idx;
        do_request(update_cnt.str());
        return out.value().at("url").c_str();
    } else {
        throw undefined_key{"key not found"};
    }
}

std::optional<pqxx::row> Database::do_request(const std::string& sql) noexcept {
    pqxx::connection database{m_uri};
    pqxx::work request{database};
    pqxx::result result{request.exec(sql)};
    request.commit();
    if (result.empty()) {
        return std::nullopt;
    } else {
        return result.at(0);
    }
}

};
