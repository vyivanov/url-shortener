cmake_minimum_required(VERSION 3.0.0)
project(url-shortener)

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

### LIBS ###

add_library(objlib OBJECT
    src/xkcd/xkcd.cpp
    src/xkcd/xkcdxx.cpp
    src/xkcd/impl.cpp
    src/xkcd/client.cpp)

target_compile_options(objlib PRIVATE -Wall -std=c++17)
target_include_directories(objlib PRIVATE include/xkcd/)

### SHARED ###

add_library(libshared SHARED $<TARGET_OBJECTS:objlib>)

set_target_properties(libshared PROPERTIES OUTPUT_NAME xkcdxx)
set_target_properties(objlib PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(libshared PRIVATE pthread ssl crypto dl)

### STATIC ###

add_library(libstatic STATIC $<TARGET_OBJECTS:objlib>)

set_target_properties(libstatic PROPERTIES OUTPUT_NAME xkcdxx)

target_link_libraries(libstatic PRIVATE pthread ssl crypto dl)

### TEST ###

add_executable(xkcd_test src/xkcd/test.c)

target_compile_options(xkcd_test PRIVATE -Wall)
target_include_directories(xkcd_test PRIVATE include/xkcd/)
target_link_libraries(xkcd_test PRIVATE libshared)

### APP ###

add_executable(${PROJECT_NAME}
    src/logger.cpp
    src/main.cpp
    src/application.cpp
    src/postgres.cpp
    src/database.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wswitch-enum -Wunreachable-code -std=c++17)
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS} pistache.a hashids.a libstatic)
