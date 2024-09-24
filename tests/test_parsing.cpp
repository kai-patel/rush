#include <gtest/gtest.h>

#include <variant>

#include "bencode.h"

TEST(BencodeParsing, PositiveInteger) {
    const auto result = bencode::parse_literal("i1234e");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_integer =
        std::holds_alternative<bencode::integer>(result.value());
    EXPECT_EQ(is_integer, true);

    const auto actual = std::get<bencode::integer>(result.value());
    const bencode::integer expected = 1234;
    EXPECT_EQ(actual, expected);
}

TEST(BencodeParsing, NegativeInteger) {
    const auto result = bencode::parse_literal("i-1234e");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_integer =
        std::holds_alternative<bencode::integer>(result.value());
    EXPECT_EQ(is_integer, true);

    const auto actual = std::get<bencode::integer>(result.value());
    const bencode::integer expected = -1234;
    EXPECT_EQ(actual, expected);
}

TEST(BencodeParsing, NonBencodeInput) {
    const auto result = bencode::parse_literal("random_text");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, false);
}

TEST(BencodeParsing, String) {
    const auto result = bencode::parse_literal("5:abcde");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_string =
        std::holds_alternative<bencode::string>(result.value());
    EXPECT_EQ(is_string, true);

    const auto actual = std::get<bencode::string>(result.value());
    const bencode::string expected = "abcde";
    EXPECT_EQ(actual, expected);
}

TEST(BencodeParsing, List) {
    const auto result = bencode::parse_literal("li123e3:fooe");

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

TEST(BencodeParsing, NestedList) {
    const auto result = bencode::parse_literal("li123e3:fooli456e3:baree");

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

TEST(BencodeParsing, Dictionary) {
    const auto result = bencode::parse_literal("d3:fooi123ee");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_dictionary =
        std::holds_alternative<bencode::dictionary>(result.value());
    EXPECT_EQ(is_dictionary, true);

    const auto actual = std::get<bencode::dictionary>(result.value());
    auto expected = bencode::dictionary{};
    expected.insert({"foo", 123});

    EXPECT_EQ(actual, expected);
}

TEST(BencodeParsing, NestedDictionary) {
    const auto result = bencode::parse_literal("d3:fooi123e3:bard3:bazi456eee");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_dictionary =
        std::holds_alternative<bencode::dictionary>(result.value());
    EXPECT_EQ(is_dictionary, true);

    const auto actual = std::get<bencode::dictionary>(result.value());
    auto expected = bencode::dictionary{};
    expected.insert({"foo", 123});

    auto expected_nested = bencode::dictionary{};
    expected_nested.insert({"baz", 456});

    expected.insert({"bar", expected_nested});

    EXPECT_EQ(actual, expected);
}

TEST(BencodeParsing, ListInNestedDictionary) {
    const auto result = bencode::parse_literal("d3:food3:barl3:bari42eeee");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_dictionary =
        std::holds_alternative<bencode::dictionary>(result.value());
    EXPECT_EQ(is_dictionary, true);

    const auto actual = std::get<bencode::dictionary>(result.value());
    auto expected_nested_list = bencode::list{};
    expected_nested_list.push_back("bar");
    expected_nested_list.push_back(42);

    auto expected_nested = bencode::dictionary{};
    expected_nested.insert({"bar", expected_nested_list});

    auto expected = bencode::dictionary{};
    expected.insert({"foo", expected_nested});

    EXPECT_EQ(actual, expected);
}

TEST(BencodeParsing, Foo) {
    const auto result = bencode::parse_literal(
        "d13:creation "
        "datei1458348895130e8:encoding5:UTF-84:infod5:filesli42eeee");

    const auto is_successful = result.has_value();
    EXPECT_EQ(is_successful, true);

    const auto is_dictionary =
        std::holds_alternative<bencode::dictionary>(result.value());
    EXPECT_EQ(is_dictionary, true);
}
