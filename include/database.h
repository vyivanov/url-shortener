#pragma once

#include <hashids.h>

#include <mutex>
#include <stdexcept>
#include <string>

namespace Shortener {

class IDatabase {
public:
    struct long_url final: std::runtime_error {
        explicit long_url(const char* what) noexcept: std::runtime_error(what) {}
    };
    struct undefined_key final: std::runtime_error {
        explicit undefined_key(const char* what) noexcept: std::runtime_error(what) {}
    };
    bool check() noexcept;
    std::string insert(const std::string& url, const std::string& ipc);
    std::string search(const std::string& key);
protected:
    explicit IDatabase(const std::string& salt) noexcept: m_key(salt, 3) {}
    virtual ~IDatabase() = default;
private:
    virtual bool do_check() = 0;
    virtual uint64_t do_insert(const std::string& url, const std::string& ipc) = 0;
    virtual std::string do_search(uint64_t idx) = 0;
private:
    std::mutex               m_mtx;
    hashidsxx::Hashids const m_key;
};

};
