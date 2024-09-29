#include <fmt/base.h>

#include <string_view>

#include "bencode.h"
#include "rush.h"
#include "torrent.h"

void parse(std::string_view filepath) {
    const auto result = torrent::from_file(std::filesystem::path{filepath});
    if (result.has_value()) {
        fmt::println("{}", result.value());
    } else {
        fmt::println("Parsing failed!!!");
    }
}

int main() {
    parse("tests/resources/bunny.torrent");
    parse("tests/resources/cosmos-laundromat.torrent");
    parse("tests/resources/lots-of-numbers.torrent");
    parse("tests/resources/tears-of-steel.torrent");
    parse("tests/resources/wired-cd.torrent");

    return 0;
}
