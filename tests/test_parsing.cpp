#include <gtest/gtest.h>

#include <variant>

#include "parser.h"

TEST(Parsing, PositiveInteger) {
    const auto result = bencode::parse("i1234e");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_integer =
        std::holds_alternative<bencode::integer>(result.value());
    EXPECT_EQ(is_integer, true);

    const auto actual = std::get<bencode::integer>(result.value());
    const bencode::integer expected = 1234;
    EXPECT_EQ(actual, expected);
}

TEST(Parsing, NegativeInteger) {
    const auto result = bencode::parse("i-1234e");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_integer =
        std::holds_alternative<bencode::integer>(result.value());
    EXPECT_EQ(is_integer, true);

    const auto actual = std::get<bencode::integer>(result.value());
    const bencode::integer expected = -1234;
    EXPECT_EQ(actual, expected);
}

TEST(Parsing, NonBencodeInput) {
    const auto result = bencode::parse("random_text");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, false);
}

TEST(Parsing, String) {
    const auto result = bencode::parse("5:abcde");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_string =
        std::holds_alternative<bencode::string>(result.value());
    EXPECT_EQ(is_string, true);

    const auto actual = std::get<bencode::string>(result.value());
    const bencode::string expected = "abcde";
    EXPECT_EQ(actual, expected);
}

TEST(Parsing, List) {
    const auto result = bencode::parse("li123e3:fooe");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_list = std::holds_alternative<bencode::list>(result.value());
    EXPECT_EQ(is_list, true);

    const auto actual = std::get<bencode::list>(result.value());
    auto expected = bencode::list{};
    expected.push_back(123);
    expected.push_back("foo");

    EXPECT_EQ(actual, expected);
}

TEST(Parsing, NestedList) {
    const auto result = bencode::parse("li123e3:fooli456e3:baree");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_list = std::holds_alternative<bencode::list>(result.value());
    EXPECT_EQ(is_list, true);

    const auto actual = std::get<bencode::list>(result.value());
    auto expected = bencode::list{};
    expected.push_back(123);
    expected.push_back("foo");

    auto expected_nested = bencode::list{};
    expected_nested.push_back(456);
    expected_nested.push_back("bar");

    expected.push_back(expected_nested);

    EXPECT_EQ(actual, expected);
}
