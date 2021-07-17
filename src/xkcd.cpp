#include "xkcd.h"

#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <string>
#include <stdexcept>
#include <sstream>

// TODO: move to separate lib

namespace {

size_t write_cb(void* const ptr, const size_t size, const size_t nmemb, std::string* const data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

};

namespace xkcdxx {

comic get_random_comic() {
    CURL* const curl = ::curl_easy_init();
    if (curl) {
        std::string body = {};
        ::curl_easy_setopt(curl, CURLOPT_URL, "http://xkcd-imgs.herokuapp.com/");
        ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        ::curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        const CURLcode res = ::curl_easy_perform(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error{::curl_easy_strerror(res)};
        }
        ::curl_easy_cleanup(curl);
        std::stringstream ss = {};
        ss << body;
        boost::property_tree::ptree json = {};
        boost::property_tree::read_json(ss, json);
        std::string title = json.get<std::string>("title");
        boost::algorithm::replace_all(title, "\x22", "&quot;");
        return comic{
            .img = json.get<std::string>("url"),
            .msg = title,
        };
    } else {
        throw std::runtime_error{"Fail to init libcurl"};
    }
}

};
