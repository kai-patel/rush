#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

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
struct fmt::formatter<torrent::single_file_info> : fmt::formatter<std::string> {
    constexpr fmt::format_context::iterator format(
        const torrent::single_file_info& t, fmt::format_context& ctx) const {
        std::string result = fmt::format(
            "[piece_length: {}, pieces: {}, private: {}, name: {}, "
            "length: {}, "
            "md5sum: {}]",
            t.piece_length, t.pieces, t.private_, t.name, t.length, t.md5sum);

        return fmt::formatter<std::string>::format(result, ctx);
    }
};

template <>
struct fmt::formatter<torrent::multifile::file> : fmt::formatter<std::string> {
    constexpr fmt::format_context::iterator format(
        const torrent::multifile::file& t, fmt::format_context& ctx) const {
        std::string result = fmt::format("[length: {}, md5sum: {}, path: {}]",
                                         t.length, t.md5sum, t.path);

        return fmt::formatter<std::string>::format(result, ctx);
    }
};

template <>
struct fmt::formatter<torrent::multi_file_info> : fmt::formatter<std::string> {
    constexpr fmt::format_context::iterator format(
        const torrent::multi_file_info& t, fmt::format_context& ctx) const {
        std::string result = fmt::format(
            "[piece_length: {}, pieces: {}, private: {}, name: {}, "
            "files: {}] ",
            t.piece_length, t.pieces, t.private_, t.name, t.files);

        return fmt::formatter<std::string>::format(result, ctx);
    }
};

template <>
struct fmt::formatter<torrent::torrent> : fmt::formatter<std::string> {
    constexpr fmt::format_context::iterator format(
        torrent::torrent t, fmt::format_context& ctx) const {
        std::string result = fmt::format(
            "[info: {}, announce: {}, announce_list: {}, creation_date: {}, "
            "comment: {}, "
            "created_by: {}, encoding: {}]",
            t.info, t.announce, t.announce_list, t.creation_date, t.comment,
            t.created_by, t.encoding);
        return fmt::formatter<std::string>::format(result, ctx);
    }
};
