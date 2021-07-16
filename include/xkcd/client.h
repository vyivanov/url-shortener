#pragma once

#include "xkcdxx.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <string>
#include <stdexcept>
#include <cstdint>

namespace xkcdxx::client {

class Xkcd final {
public:
    Xkcd() noexcept;
    Xkcd(const Xkcd& other) = delete;
    Xkcd(Xkcd&& other) = delete;
    Xkcd& operator=(const Xkcd& other) = delete;
    Xkcd& operator=(Xkcd&& other) = delete;
    Comic::Info get_info(uint64_t num);
private:
    httplib::Client m_xkcd;
};

};
