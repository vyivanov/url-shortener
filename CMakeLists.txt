cmake_minimum_required(VERSION 3.0.0)
project(url-shortener)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(SRCS main.cpp application.cpp database.cpp)

add_executable(${PROJECT_NAME} ${SRCS})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -std=c++17)

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS} hashids)
