#pragma once

#include <fmt/format.h>

#include <filesystem>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>

#include "bencode.h"

namespace torrent {

struct single_file_info {
    bencode::integer piece_length;
    bencode::string pieces;
    std::optional<bencode::integer> private_;

    bencode::string name;
    bencode::integer length;
    std::optional<bencode::string> md5sum;
};

namespace multifile {
struct file {
    bencode::integer length;
    bencode::string md5sum;
    bencode::string path;
};
}  // namespace multifile

struct multi_file_info {
    bencode::integer piece_length;
    bencode::string pieces;
    std::optional<bencode::integer> private_;

    bencode::string name;
    std::vector<multifile::file> files;
};

struct torrent {
    std::variant<single_file_info, multi_file_info> info;
    bencode::string announce;
    std::optional<bencode::list> announce_list;
    std::optional<bencode::integer> creation_date;
    std::optional<bencode::string> comment;
    std::optional<bencode::string> created_by;
    std::optional<bencode::string> encoding;
};

std::optional<torrent> from_file(const std::filesystem::path& path);
}  // namespace torrent

template <>
struct fmt::formatter<torrent::torrent> : fmt::formatter<std::string> {
    constexpr fmt::format_context::iterator format(
        torrent::torrent t, fmt::format_context& ctx) const {
        const auto value_or_null = []<typename T>(std::optional<T> v) {
            if (v.has_value()) {
                return fmt::format("{}", v.value());
            }
            return std::string{"(nullopt)"};
        };

        std::string result = fmt::format(
            "[announce: {}, announce_list: {}, creation_date: {}, comment: {}, "
            "created_by: {}, encoding: {}]",
            t.announce, value_or_null(t.announce_list),
            value_or_null(t.creation_date), value_or_null(t.comment),
            value_or_null(t.created_by), value_or_null(t.encoding));
        return fmt::formatter<std::string>::format(result, ctx);
    }
};
