cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "build type")

project(lfw LANGUAGES CXX)

add_executable(librefw)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

pkg_check_modules(NL REQUIRED IMPORTED_TARGET libnl-genl-3.0)
target_compile_options(librefw PRIVATE $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -Werror>)

target_sources(librefw PRIVATE
    src/main.cpp
    src/util/nl.cpp
    src/nl/log.cpp
    src/nl/base.cpp
)

target_include_directories(librefw PRIVATE include/)
target_link_libraries(librefw PRIVATE
    Threads::Threads
    CLI11::CLI11
    PkgConfig::NL
)

install(TARGETS librefw RUNTIME)
