#include "application.h"

#include <iostream>

int main()
{

    // pqxx::connection db{"postgresql://user:pswd@127.0.0.1/shortener"};
    // pqxx::work wrk{db};

    // pqxx::result res{wrk.exec("SELECT * FROM item")};
    // wrk.commit();
    // for (const auto row: res) {
    //     std::cout
    //         << row["idx"].c_str() << '\x20'
    //         << row["url"].c_str() << '\x20'
    //         << row["tms"].c_str() << '\x20'
    //         << row["ipc"].c_str() << '\x20'
    //         << row["cnt"].c_str() << '\x20'
    //         << '\n';
    // }

    Shortener::Application().serve();
}
