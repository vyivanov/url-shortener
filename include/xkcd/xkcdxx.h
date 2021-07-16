#pragma once

#include <stdexcept>
#include <string>
#include <cstdint>
#include <ctime>
#include <memory>

namespace xkcdxx {

class Comic final {
public:
    enum class Number {
        Latest, Random
    };
    struct request_failed final: std::runtime_error {
        explicit request_failed(const std::string& why) noexcept;
    };
    explicit Comic(Comic::Number number);
    explicit Comic(uint32_t number);
    Comic(const Comic& other) = delete;
    Comic(Comic&& other) = delete;
    Comic& operator=(const Comic& other) = delete;
    Comic& operator=(Comic&& other) = delete;
    struct Info {
        uint32_t    number;
        std::time_t date;
        std::string url;
        std::string title;
        std::string alt;
    };
    const Comic::Info& info() const noexcept;
   ~Comic();
private:
    class Impl;
    std::unique_ptr<Impl> m_pimpl;
};

};
