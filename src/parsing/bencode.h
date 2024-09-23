#pragma once

#include <fmt/format.h>
#include <fmt/base.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace bencode {

using integer = std::int64_t;
using string = std::string;
struct list;
struct dictionary;

using value = std::variant<integer, string, list, dictionary>;

struct list : std::vector<value> {};
struct dictionary : std::unordered_map<string, value> {};

std::optional<bencode::value> parse_literal(std::string_view input);
std::optional<bencode::value> parse(const std::filesystem::path& input);

struct value_printer {
    std::string operator()(integer i);
    std::string operator()(string s);
    std::string operator()(list l);
    std::string operator()(dictionary d);
};
}  // namespace bencode

template <>
struct fmt::formatter<bencode::value> : fmt::formatter<std::string> {
    constexpr fmt::format_context::iterator format(
        bencode::value v, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(
            std::visit(bencode::value_printer{}, v), ctx);
    }
};

template <>
struct fmt::formatter<bencode::list> : fmt::formatter<std::string> {
    constexpr fmt::format_context::iterator format(
        bencode::list v, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(bencode::value_printer{}(v),
                                                   ctx);
    }
};

template <>
struct fmt::formatter<bencode::dictionary> : fmt::formatter<std::string> {
    constexpr fmt::format_context::iterator format(
        const bencode::dictionary& v, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(bencode::value_printer{}(v),
                                                   ctx);
    }
};
