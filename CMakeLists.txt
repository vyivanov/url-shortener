cmake_minimum_required(VERSION 3.0.0)
project(url-shortener)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(${PROJECT_NAME} src/main.cpp src/application.cpp src/database.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -std=c++17)
target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS} hashids)
