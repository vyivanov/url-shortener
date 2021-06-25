cmake_minimum_required(VERSION 3.0.0)
project(url-shortener)

add_compile_options(-Wall -std=c++17)
set(SRCS main.cpp)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} -lpqxx -lpq -lpistache)
