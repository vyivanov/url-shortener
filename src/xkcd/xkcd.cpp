#include "xkcd.h"
#include "xkcdxx.h"

#include <string>
#include <memory>

namespace {

thread_local std::string latest_error = {};

template <typename T, typename I>
constexpr T to(const I input) noexcept {
    return reinterpret_cast<T>(input);
}

template <typename T>
xkcd_comic comic(const T number) noexcept {
    xkcd_comic ret = XKCD_COMIC_NULL;
    try {
        const auto ptr = std::make_unique<xkcdxx::Comic>(number).release();
        const auto idx = ::to<xkcd_comic>(ptr);
        ret = idx;
    } catch (const xkcdxx::Comic::request_failed& ex) {
        ::latest_error = ex.what();
    }
    return ret;
}

};

xkcd_comic xkcd_comic_latest() {
    return ::comic(xkcdxx::Comic::Number::Latest);
}

xkcd_comic xkcd_comic_random() {
    return ::comic(xkcdxx::Comic::Number::Random);
}

xkcd_comic xkcd_comic_exact(const uint32_t number) {
    return ::comic(number);
}

const char* xkcd_comic_error() {
    return ::latest_error.c_str();
}

xkcd_info xkcd_comic_info(const xkcd_comic idx) {
    const auto comic = ::to<xkcdxx::Comic*>(idx);
    const auto info  = xkcd_info {
        .number = comic->info().number       ,
        .date   = comic->info().date         ,
        .url    = comic->info().url  .c_str(),
        .title  = comic->info().title.c_str(),
        .alt    = comic->info().alt  .c_str(),
    };
    return info;
}

void xkcd_comic_destroy(const xkcd_comic idx) {
    const auto ptr = ::to<xkcdxx::Comic*>(idx);
    const auto del = std::unique_ptr<xkcdxx::Comic>(ptr);
}
