#pragma once

#include <hashids.h>
#include <pqxx/pqxx>

#include <string>
#include <stdexcept>

namespace Shortener {

class Database final {
public:
    Database(const std::string& uri, const std::string& salt) noexcept;
    Database(const Database& rh) = delete;
    Database(Database&& rh) = delete;
    Database& operator=(const Database& rh) = delete;
    Database& operator=(Database&& rh) = delete;
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
    pqxx::result do_request(const std::string& sql) noexcept;
private:
    const std::string        m_uri;
    const hashidsxx::Hashids m_key;
};

};
