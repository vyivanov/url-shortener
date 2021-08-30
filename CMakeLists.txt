cmake_minimum_required(VERSION 3.16.0)
project(url-shortener LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

execute_process(
    COMMAND bash -c "git describe --tags --abbrev=0 | tr -d '\n'"
    OUTPUT_VARIABLE APP_SEMVER)

if("${APP_SEMVER}" STREQUAL "")
    message(FATAL_ERROR "Fail to derive APP_SEMVER")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h)

add_executable(${PROJECT_NAME}
    src/logger.cpp
    src/main.cpp
    src/application.cpp
    src/postgres.cpp
    src/database.cpp)

set(COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wpedantic
    -Wswitch-enum
    -Wunreachable-code
    -Wunused
    -pipe
    -Ofast
    -std=c++2a)

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS})
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_link_options(${PROJECT_NAME} PRIVATE "-s")
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS} pistache.a hashids.a xkcdxx.a)
