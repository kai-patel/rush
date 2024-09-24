add_executable(
    rush
    rush.cpp
    main.cpp
)

target_link_libraries(
    rush
    PRIVATE
    parsing
    torrent
    Boost::boost
    fmt::fmt
    tomlplusplus::tomlplusplus
)

