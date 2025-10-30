cmake_minimum_required(VERSION 3.28)

project(librefw LANGUAGES CXX)

add_executable(librefw)

target_sources(librefw PRIVATE src/main.cpp)
