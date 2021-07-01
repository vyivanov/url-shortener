#pragma once

#include <pqxx/pqxx>

#include <string>
#include <string_view>
#include <stdexcept>

namespace Shortener {

class Database final {
public:
    explicit Database(const std::string_view uri) noexcept;
    Database(const Database& rh) = delete;
    Database(Database&& rh) = delete;
    Database& operator=(const Database& rh) = delete;
    Database& operator=(Database&& rh) = delete;
    std::string insert(std::string url, const std::string_view ipc) noexcept;
    std::string search(const std::string_view key);
    class undefined_key final : public std::runtime_error {
    public:
        explicit undefined_key(const char* what) noexcept : std::runtime_error{what} {;;;}
    };
private:
    pqxx::result do_request(const std::string_view sql);
private:
    const std::string m_uri;
};

}
