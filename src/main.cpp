#include <fmt/base.h>
#include <fmt/core.h>

#include <cstddef>
#include <cstdint>
#include <lexy/action/match.hpp>
#include <lexy/action/parse.hpp>
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
#include <lexy/dsl/parse_as.hpp>
#include <lexy/dsl/scan.hpp>
#include <lexy/dsl/sign.hpp>
#include <lexy/encoding.hpp>
#include <lexy/error.hpp>
#include <lexy/grammar.hpp>
#include <lexy/input/buffer.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "rush.h"

template <class... Ts>
struct visitor : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
visitor(Ts...) -> visitor<Ts...>;

namespace bencode {

using integer = std::int64_t;
using string = std::string;
struct list;
struct dictionary;

using value = std::variant<integer, string, list, dictionary>;

struct list : std::vector<value> {};
struct dictionary : std::unordered_map<string, value> {};

struct ValuePrinter {
    std::string operator()(integer i) { return std::to_string(i); }

    std::string operator()(string s) { return s; }

    std::string operator()(list l) {
        auto result = std::stringstream{};
        result << '[';

        std::size_t i = 0;
        const auto last_index = l.size() - 1;

        for (const auto& element : l) {
            const auto printed_element = std::visit(ValuePrinter{}, element);
            result << printed_element;
            if (i < last_index) {
                result << ", ";
            }
            i++;
        }

        result << ']';
        return result.str();
    }

    std::string operator()(dictionary d) {
        auto result = std::stringstream{};
        result << '{';

        std::size_t i = 0;
        const auto last_index = d.size() - 1;

        for (const auto& [key, element] : d) {
            const auto printed_element = std::visit(ValuePrinter{}, element);
            result << key << ": " << printed_element;
            if (i < last_index) {
                result << ", ";
            }
            i++;
        }

        result << '}';
        return result.str();
    }
};

auto format_as(value v) {
    return std::visit(ValuePrinter{}, v);
}

auto format_as(list l) {
    return ValuePrinter{}(l);
}

auto format_as(dictionary d) {
    return ValuePrinter{}(d);
}

namespace grammar {
struct integer {
    static constexpr auto rule =
        lexy::dsl::lit_c<'i'> >> lexy::dsl::minus_sign +
                                     lexy::dsl::integer<std::int64_t> +
                                     lexy::dsl::lit_c<'e'>;
    static constexpr auto value =
        lexy::as_integer<std::int64_t> | lexy::construct<bencode::integer>;
};

struct byte_string
    : lexy::scan_production<lexy::buffer_lexeme<lexy::byte_encoding>>,
      lexy::token_production {
    template <typename Context, typename Reader>
    static constexpr scan_result scan(
        lexy::rule_scanner<Context, Reader>& scanner) {
        lexy::scan_result<std::size_t> length;
        scanner.parse(length, lexy::dsl::integer<std::size_t>);
        scanner.parse(lexy::dsl::lit_c<':'>);

        if (!scanner) {
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

    static constexpr auto value =
        lexy::callback<bencode::string>([](scan_result::value_type buffer) {
            return std::string(reinterpret_cast<const char*>(buffer.data()),
                               buffer.size());
        });
};

struct list {
    static constexpr auto rule = [] {
        auto open = lexy::dsl::lit_c<'l'>;
        auto bencode_integer = lexy::dsl::p<integer>;
        auto bencode_string = lexy::dsl::p<byte_string>;
        auto bencode_list = lexy::dsl::recurse_branch<list>;
        auto close = lexy::dsl::lit_c<'e'>;
        auto content_integer =
            lexy::dsl::peek(bencode_integer) >> bencode_integer;
        auto content_string = lexy::dsl::peek(bencode_string) >> bencode_string;
        auto content_list = lexy::dsl::peek(open) >> bencode_list;

        auto content =
            lexy::dsl::list(content_integer | content_string | content_list);

        return open >> content + close;
    }();

    static constexpr auto value = lexy::as_list<bencode::list>;
};

struct dictionary {
    static constexpr auto rule = [] {
        auto open = lexy::dsl::lit_c<'d'>;
        auto close = lexy::dsl::lit_c<'e'>;
        auto bencode_integer = lexy::dsl::p<integer>;
        auto bencode_string = lexy::dsl::p<byte_string>;
        auto bencode_list = lexy::dsl::p<list>;
        auto bencode_dict = lexy::dsl::recurse_branch<dictionary>;
        auto content_integer =
            lexy::dsl::peek(bencode_integer) >> bencode_integer;
        auto content_string = lexy::dsl::peek(bencode_string) >> bencode_string;
        auto content_list = lexy::dsl::peek(bencode_list) >> bencode_list;
        auto content_dict = lexy::dsl::peek(open) >> bencode_dict;

        auto content =
            lexy::dsl::list(content_string + (content_integer | content_string |
                                              content_list | content_dict));

        return open >> content + close;
    }();

    static constexpr auto value = lexy::as_collection<bencode::dictionary>;
};
}  // namespace grammar
}  // namespace bencode

template <typename Production>
void test_parse(std::string_view input) {
    fmt::println("Attempting to parse: {}", input);
    const auto literal = lexy::zstring_input<lexy::byte_encoding>(input.data());
    const auto result =
        lexy::parse<Production>(literal, lexy_ext::report_error);

    if (result.has_value()) {
        const auto value = result.value();
        fmt::println("Parsed: {}", value);
    }
}

int main() {
    test_parse<bencode::grammar::integer>("i-1234e");
    test_parse<bencode::grammar::byte_string>("5:a cde");
    test_parse<bencode::grammar::list>("li-1234e5:abcdel3:fooee");
    test_parse<bencode::grammar::dictionary>("d3:food5:abcdei-42eee");

    return 0;
}
