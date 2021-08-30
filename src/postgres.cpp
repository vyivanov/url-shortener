#include "postgres.h"
#include "logger.h"

#include <fmt/core.h>
#include <boost/lexical_cast.hpp>
#include <pqxx/pqxx>

#include <stdexcept>
#include <string>
#include <optional>

namespace {

constexpr const char* TMPL_SEARCH_URL = "SELECT idx FROM public.item WHERE url = '{}' LIMIT 1";
constexpr const char* TMPL_INSERT_URL = "INSERT INTO public.item (url, ipc) VALUES('{}', '{}') RETURNING idx";
constexpr const char* TMPL_SEARCH_IDX = "SELECT url FROM public.item WHERE idx = {}";
constexpr const char* TMPL_UPDATE_CNT = "UPDATE public.item SET cnt = cnt + 1 WHERE idx = {}";

}

namespace Shortener {

bool Postgres::do_ping() noexcept {
    try {
        do_request("SELECT idx FROM public.item LIMIT 1");
    } catch (const std::exception& ex) {
        LOG_ERROR(ex.what());
        return false;
    }
    return true;
}

uint64_t Postgres::do_insert(const std::string& url, const std::string& ipc) {
    const auto idx = [&]() -> std::string {
        const auto search_url = fmt::format(TMPL_SEARCH_URL, url);
        if (const auto out = do_request(search_url); out.has_value()) {
            return out.value().at("idx").c_str();
        }
        const auto insert_url = fmt::format(TMPL_INSERT_URL, url, ipc);
        const auto out = do_request(insert_url);
        return out.value().at("idx").c_str();
    }();
    return boost::lexical_cast<int>(idx);
}

std::string Postgres::do_search(const uint64_t idx) {
    const auto search_idx = fmt::format(TMPL_SEARCH_IDX, idx);
    if (const auto out = do_request(search_idx); out.has_value()) {
        const auto update_cnt = fmt::format(TMPL_UPDATE_CNT, idx);
        do_request(update_cnt);
        return out.value().at("url").c_str();
    } else {
        throw IDatabase::undefined_key{"key not found"};
    }
}

std::optional<pqxx::row> Postgres::do_request(const std::string& sql) {
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

}
