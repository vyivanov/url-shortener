#pragma once

#include "xkcdxx.h"
#include <cstdint>

namespace xkcdxx {

class Comic::Impl final {
public:
    explicit Impl(Comic::Number number);
    explicit Impl(uint32_t number);
    const Comic::Info& info() const noexcept;
private:
    static uint32_t to_int(Comic::Number number);
private:
    const Comic::Info m_info;
};

};
