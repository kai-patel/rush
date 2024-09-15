cmake_minimum_required(VERSION 3.15)
project(rush CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,bugprone-*,misc-const-correctness,readability-make-member-function-const)

find_package(Boost REQUIRED)
find_package(lexy REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(src)

install(
    TARGETS rush
    DESTINATION "."
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
