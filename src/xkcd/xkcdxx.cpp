#include "xkcdxx.h"
#include "impl.h"

#include <string>
#include <stdexcept>
#include <memory>
#include <cstdint>

namespace xkcdxx {

Comic::request_failed::request_failed(const std::string& why) noexcept
    : std::runtime_error(why) {
}

Comic::Comic(const Comic::Number number)
    : m_pimpl(std::make_unique<Comic::Impl>(number)) {
}

Comic::Comic(const uint32_t number)
    : m_pimpl(std::make_unique<Comic::Impl>(number)) {
}

const Comic::Info& Comic::info() const noexcept {
    return m_pimpl->info();
}

Comic::~Comic() = default;

};
