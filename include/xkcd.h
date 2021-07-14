#pragma once

#include <string>

namespace xkcdxx {

struct comic {
    std::string img;
    std::string msg;
};

comic get_random_comic();

};
