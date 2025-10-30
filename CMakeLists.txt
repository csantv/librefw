cmake_minimum_required(VERSION 3.28)
project(lfw LANGUAGES CXX)

add_executable(librefw)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(NL REQUIRED IMPORTED_TARGET libnl-3.0)

target_sources(librefw PRIVATE src/main.cpp)
target_include_directories(librefw PRIVATE include/)
target_link_libraries(librefw PRIVATE Threads::Threads PkgConfig::NL)
