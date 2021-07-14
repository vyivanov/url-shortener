#pragma once

#include <hashids.h>
#include <pqxx/pqxx>

#include <string>
#include <stdexcept>
#include <mutex>
#include <optional>

namespace Shortener {

class Postgres final {
public:
    Postgres(const std::string& uri, const std::string& salt) noexcept;
    Postgres(const Postgres& rh) = delete;
    Postgres(Postgres&& rh) = delete;
    Postgres& operator=(const Postgres& rh) = delete;
    Postgres& operator=(Postgres&& rh) = delete;
    std::string insert(const std::string& url, const std::string& ipc);
    std::string search(const std::string& key);
    class long_url final : public std::runtime_error {
    public:
        explicit long_url(const char* what) noexcept : std::runtime_error{what} {;;;}
    };
    class undefined_key final : public std::runtime_error {
    public:
        explicit undefined_key(const char* what) noexcept : std::runtime_error{what} {;;;}
    };
private:
    std::optional<pqxx::row> do_request(const std::string& sql) noexcept;
private:
    std::string const        m_uri;
    hashidsxx::Hashids const m_key;
    std::mutex               m_mtx;
};

};
