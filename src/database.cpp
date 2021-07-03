#include "database.h"

#include <pqxx/pqxx>
#include <boost/format.hpp>

#include <string>
#include <vector>

namespace Shortener {

constexpr const char* SQL_SELECT_TABLE = "SELECT * FROM public.item";
constexpr const char* SQL_CREATE_TABLE =
R"(

CREATE TABLE public.item
(
    idx serial NOT NULL,
    url text NOT NULL,
    tms timestamp NOT NULL DEFAULT timezone('UTC'::text, now()),
    ipc inet NULL,
    cnt int NOT NULL DEFAULT 0,
    CONSTRAINT item_pk PRIMARY KEY (idx)
);

COMMENT ON COLUMN public.item.idx IS 'key to be provided to client';
COMMENT ON COLUMN public.item.url IS 'link to be redirected to';
COMMENT ON COLUMN public.item.tms IS 'utc timestamp the record was created at';
COMMENT ON COLUMN public.item.ipc IS 'client ip the record was created from';
COMMENT ON COLUMN public.item.cnt IS 'redirection counter';

)";

constexpr const char* TMPL_INSERT_ITEM = "INSERT INTO public.item (url, ipc) VALUES('%1%', '%2%') RETURNING idx";
constexpr const char* TMPL_SEARCH_ITEM = "UPDATE public.item SET cnt = cnt + 1 WHERE idx = %1%;"
                                         "SELECT url FROM public.item WHERE idx = %1%";

Database::Database(const std::string& uri) noexcept : m_uri{uri}, m_key{"test", 3} {
    try {
        do_request(SQL_SELECT_TABLE);
    } catch (const pqxx::undefined_table&) {
        do_request(SQL_CREATE_TABLE);
    }
}

std::string Database::insert(const std::string& url, const std::string& ipc) noexcept {
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

pqxx::result Database::do_request(const std::string& sql) {
    pqxx::connection database{m_uri};
    pqxx::work request{database};
    pqxx::result result{request.exec(sql)};
    request.commit();
    return result;
}

}
