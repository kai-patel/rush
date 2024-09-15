#include <fmt/base.h>

#include "bencode.h"
#include "rush.h"
#include "torrent.h"

int main() {
    const auto result = torrent::from_file(
        std::filesystem::path{"tests/resources/alice.torrent"});
    if (result.has_value()) {
        fmt::println("{}", result.value());
    }
    return 0;
}
