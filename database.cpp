#include "database.h"

#include <pqxx/pqxx>
#include <string_view>

namespace Shortener {

constexpr const char* SELECT_TABLE = "SELECT * FROM item";
constexpr const char* CREATE_TABLE =
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

Database::Database(const std::string_view uri) : m_uri{uri} {
    pqxx::connection db{m_uri};
    try {
        pqxx::work test{db};
        test.exec(SELECT_TABLE);
    } catch (const pqxx::undefined_table& e) {
        pqxx::work init{db};
        init.exec(CREATE_TABLE);
        init.commit();
    }
}

}
