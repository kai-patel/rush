add_library(
    parsing
    STATIC
    bencode.cpp
)

target_link_libraries(
    parsing
    PUBLIC
    fmt::fmt
    PRIVATE
    foonathan::lexy
)

target_include_directories(
    parsing
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
