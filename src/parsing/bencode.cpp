#include "bencode.h"

#include <fmt/base.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <lexy/action/match.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/action/trace.hpp>
#include <lexy/callback.hpp>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/composition.hpp>
#include <lexy/callback/integer.hpp>
#include <lexy/callback/object.hpp>
#include <lexy/dsl.hpp>
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/byte.hpp>
#include <lexy/dsl/context_counter.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/integer.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/parse_as.hpp>
#include <lexy/dsl/scan.hpp>
#include <lexy/dsl/sign.hpp>
#include <lexy/encoding.hpp>
#include <lexy/error.hpp>
#include <lexy/grammar.hpp>
#include <lexy/input/buffer.hpp>
#include <lexy/input/file.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

template <class... Ts>
struct visitor : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
visitor(Ts...) -> visitor<Ts...>;

namespace bencode {
std::string value_printer::operator()(integer i) {
    return std::to_string(i);
}

std::string value_printer::operator()(string s) {
    return s;
}

std::string value_printer::operator()(list l) {
    auto result = std::stringstream{};
    result << '[';

    std::size_t i = 0;
    const auto last_index = l.size() - 1;

    for (const auto& element : l) {
        const auto printed_element = std::visit(value_printer{}, element);
        result << printed_element;
        if (i < last_index) {
            result << ", ";
        }
        i++;
    }

    result << ']';
    return result.str();
}

std::string value_printer::operator()(dictionary d) {
    auto result = std::stringstream{};
    result << '{';

    std::size_t i = 0;
    const auto last_index = d.size() - 1;

    for (const auto& [key, element] : d) {
        const auto printed_element = std::visit(value_printer{}, element);
        result << key << ": " << printed_element;
        if (i < last_index) {
            result << ", ";
        }
        i++;
    }

    result << '}';
    return result.str();
}

template <typename Production>
std::optional<bencode::value> parse_literal(std::string_view input) {
    const auto literal = lexy::zstring_input<lexy::byte_encoding>(input.data());
    const auto result =
        lexy::parse<Production>(literal, lexy_ext::report_error);

    if (result.has_value()) {
        const auto value = result.value();
        return bencode::value{value};
    }
    return {};
}

namespace grammar {
struct integer {
    static constexpr auto rule = [] {
        const auto open = lexy::dsl::lit_c<'i'>;
        const auto close = lexy::dsl::lit_c<'e'>;
        const auto number =
            lexy::dsl::minus_sign + lexy::dsl::integer<std::int64_t>;

        return (open >> number) + close;
    }();

    static constexpr auto value = lexy::as_integer<bencode::integer>;
};

struct byte_string
    : lexy::scan_production<lexy::buffer_lexeme<lexy::byte_encoding>> {
    template <typename Context, typename Reader>
    static constexpr scan_result scan(
        lexy::rule_scanner<Context, Reader>& scanner) {
        lexy::scan_result<std::size_t> length;
        if (!scanner.branch(length, lexy::dsl::integer<std::size_t>)) {
            return lexy::scan_failed;
        }

        if (!scanner.branch(lexy::dsl::lit_c<':'>)) {
            return lexy::scan_failed;
        }

        const auto n = length.value();
        auto start = scanner.position();
        for (int i = 0; i < n; i++) {
            if (scanner.is_at_eof()) {
                scanner.fatal_error(lexy::expected_char_class{},
                                    scanner.position(), "byte");
                lexy::error<Reader, void>(scanner.position(), "e");
            }
            scanner.parse(lexy::dsl::byte);
            if (!scanner) {
                return lexy::scan_failed;
            }
        }

        if (!scanner) {
            return lexy::scan_failed;
        }

        auto end = scanner.position();

        return scan_result::value_type(start, end);
    }

    static constexpr auto rule =
        (lexy::dsl::peek(lexy::dsl::integer<std::size_t>)) >> lexy::dsl::scan;

    static constexpr auto value =
        lexy::callback<bencode::string>([](scan_result::value_type buffer) {
            return std::string(reinterpret_cast<const char*>(buffer.data()),
                               buffer.size());
        });
};

struct dictionary;

struct list {
    static constexpr auto rule = [] {
        const auto open = lexy::dsl::lit_c<'l'>;
        const auto close = lexy::dsl::lit_c<'e'>;
        const auto elements =
            lexy::dsl::list(lexy::dsl::p<integer> | lexy::dsl::p<byte_string> |
                            lexy::dsl::recurse_branch<list> |
                            lexy::dsl::recurse_branch<dictionary>);

        return (open >> elements) + close;
    }();

    static constexpr auto value = lexy::as_list<bencode::list>;
};

struct dictionary {
    static constexpr auto rule = [] {
        const auto open = lexy::dsl::lit_c<'d'>;
        const auto close = lexy::dsl::lit_c<'e'>;
        const auto elements = lexy::dsl::list(
            lexy::dsl::p<byte_string> >>
            (lexy::dsl::p<integer> | lexy::dsl::p<byte_string> |
             lexy::dsl::p<list> | lexy::dsl::recurse_branch<dictionary>));

        return (open >> elements) + close;
    }();

    static constexpr auto value = lexy::as_collection<bencode::dictionary>;
};

struct bencode_value {
    static constexpr auto rule = [] {
        return lexy::dsl::p<integer> | lexy::dsl::p<byte_string> |
               lexy::dsl::p<list> | lexy::dsl::p<dictionary>;
    }();

    static constexpr auto value = lexy::construct<bencode::value>;
};
}  // namespace grammar

std::optional<bencode::value> parse_literal(std::string_view input) {
    return parse_literal<bencode::grammar::bencode_value>(input);
}

std::optional<bencode::value> parse(const std::filesystem::path& input) {
    const auto file = lexy::read_file<lexy::byte_encoding>(input.c_str());
    if (!file) {
        fmt::println("Could not open file: {} - error {}",
                     std::filesystem::absolute(input).string(),
                     static_cast<int>(file.error()));
        return {};
    }

    // lexy::trace<bencode::grammar::bencode_value>(stdout, file.buffer(),
    //                                              {lexy::visualize_fancy});
    const auto result = lexy::parse<bencode::grammar::bencode_value>(
        file.buffer(), lexy_ext::report_error);

    if (result.has_value()) {
        const auto value = result.value();
        return bencode::value{value};
    }
    return {};
}

}  // namespace bencode
