add_executable(
    rush
    rush.cpp
    main.cpp
)

target_link_libraries(
    rush
    PRIVATE
    parsing
    Boost::boost
    fmt::fmt
)

