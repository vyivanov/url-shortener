#pragma once

#include <string>
#include <string_view>

namespace Shortener {

class Database final {
public:
    explicit Database(std::string_view uri);
    Database(const Database& rh) = delete;
    Database(Database&& rh) = delete;
   ~Database() = default;
    Database& operator=(const Database& rh) = delete;
    Database& operator=(Database&& rh) = delete;
private:
    const std::string m_uri;
};

}
