add_library(
    torrent
    STATIC
    torrent.cpp
)

target_link_libraries(
    torrent
    PRIVATE
    parsing
    fmt::fmt
)

target_include_directories(
    torrent
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
