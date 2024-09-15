enable_testing()

find_package(GTest REQUIRED)

add_executable(
    test_parsing
    test_parsing.cpp
)

target_link_libraries(
    test_parsing
    PRIVATE
    gtest::gtest
    parsing
    fmt::fmt
)

include(GoogleTest)
gtest_discover_tests(
    test_parsing
)


