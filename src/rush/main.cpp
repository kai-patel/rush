#include <fmt/base.h>

#include "parser.h"
#include "rush.h"

int main() {
    bencode::parse("i-1234e");
    bencode::parse("5:a cde");
    bencode::parse("li-1234e5:abcdel3:fooee");
    bencode::parse("d3:food5:abcdei-42eee");
    fmt::println("{}", bencode::parse("d3:food5:abcdei-42eee").value());

    return 0;
}
