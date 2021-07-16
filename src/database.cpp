#include "database.h"

#include <mutex>
#include <string>

namespace {

constexpr uint16_t MAX_URL_LENGTH = 1'024UL;

};

namespace Shortener {

bool IDatabase::ping() noexcept {
    std::lock_guard<std::mutex> lg{m_mtx};
    return do_ping();
}

std::string IDatabase::insert(const std::string& url, const std::string& ipc) {
    if (url.length() > MAX_URL_LENGTH) {
        throw long_url{"url is too long"};
    }
    std::lock_guard<std::mutex> lg{m_mtx};
    const uint64_t idx = do_insert(url, ipc);
    return m_key.encode(idx);
}

std::string IDatabase::search(const std::string& key) {
    const auto idx = [&]() -> uint64_t {
        const std::vector<uint64_t> idx = m_key.decode(key);
        if (idx.empty()) {
            throw undefined_key{"key not found"};
        } else {
            return idx.at(0);
        }
    }();
    std::lock_guard<std::mutex> lg{m_mtx};
    const std::string url = do_search(idx);
    return url;
}

};
