#include "impl.h"
#include "xkcdxx.h"
#include "client.h"

#include <cstdint>
#include <random>

namespace xkcdxx {

Comic::Impl::Impl(const Comic::Number number)
    : m_info(
        client::Xkcd().get_info(to_int(number))
    ) {
}

Comic::Impl::Impl(const uint32_t number)
    : m_info(
        client::Xkcd().get_info(number)
    ) {
}

const Comic::Info& Comic::Impl::info() const noexcept {
    return m_info;
}

uint32_t Comic::Impl::to_int(const Comic::Number number) {
    if (number == Comic::Number::Latest) {
        return 0;
    } else if (number == Comic::Number::Random) {
        auto rnd = std::random_device();
        auto gen = std::mt19937(rnd());
        auto end = client::Xkcd().get_info(to_int(Comic::Number::Latest)).number - 1;
        auto uid = std::uniform_int_distribution<uint32_t>(1, end);
        return uid(gen);
    }
    return 0;
}

};
