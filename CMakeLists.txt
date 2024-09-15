cmake_minimum_required(VERSION 3.15)
project(rush CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,bugprone-*,misc-const-correctness,readability-make-member-function-const)

find_package(Boost)
find_package(lexy)
find_package(fmt)

add_executable(
    rush
    src/rush.cpp
    src/main.cpp
    src/parser.cpp
)

target_link_libraries(
    rush
    PRIVATE
    Boost::boost
    foonathan::lexy
    fmt::fmt
)

install(
    TARGETS rush
    DESTINATION "."
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
