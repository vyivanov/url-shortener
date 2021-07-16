#pragma once

#include "database.h"

#include <pqxx/pqxx>

#include <string>
#include <optional>

namespace Shortener {

class Postgres final: public IDatabase {
public:
    Postgres(const std::string& uri, const std::string& salt) noexcept: IDatabase(salt), m_uri(uri) {}
    Postgres(const Postgres& rh) = delete;
    Postgres(Postgres&& rh) = delete;
    Postgres& operator=(const Postgres& rh) = delete;
    Postgres& operator=(Postgres&& rh) = delete;
private:
    bool do_ping() noexcept override;
    uint64_t do_insert(const std::string& url, const std::string& ipc) override;
    std::string do_search(uint64_t idx) override;
private:
    std::optional<pqxx::row> do_request(const std::string& sql);
private:
    const std::string m_uri;
};

};
