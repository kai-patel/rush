#include "torrent.h"

#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "bencode.h"

namespace torrent {
std::optional<std::variant<single_file_info, multi_file_info>> process_info(
    const bencode::dictionary& info) {
    return {};
}

std::optional<torrent> torrent_from_dictionary(
    const bencode::dictionary& contents) {
    torrent new_torrent{};
    if (contents.contains("info")) {
        const auto element = contents.find("info");
        if (std::holds_alternative<bencode::dictionary>(element->second)) {
            const auto info =
                process_info(std::get<bencode::dictionary>(element->second));
            if (info.has_value()) {
                new_torrent.info = info.value();
            }
        }
    }

    if (contents.contains("announce")) {
        const auto element = contents.find("announce");
        if (std::holds_alternative<bencode::string>(element->second)) {
            new_torrent.announce = std::get<bencode::string>(element->second);
        }
    }

    if (contents.contains("announce-list")) {
        const auto element = contents.find("announce-list");
        if (std::holds_alternative<bencode::list>(element->second)) {
            new_torrent.announce_list =
                std::get<bencode::list>(element->second);
        }
    }

    if (contents.contains("creation date")) {
        const auto element = contents.find("creation date");
        if (std::holds_alternative<bencode::integer>(element->second)) {
            new_torrent.creation_date =
                std::get<bencode::integer>(element->second);
        }
    }

    if (contents.contains("comment")) {
        const auto element = contents.find("comment");
        if (std::holds_alternative<bencode::string>(element->second)) {
            new_torrent.comment = std::get<bencode::string>(element->second);
        }
    }

    if (contents.contains("created by")) {
        const auto element = contents.find("created by");
        if (std::holds_alternative<bencode::string>(element->second)) {
            new_torrent.created_by = std::get<bencode::string>(element->second);
        }
    }

    if (contents.contains("encoding")) {
        const auto element = contents.find("encoding");
        if (std::holds_alternative<bencode::string>(element->second)) {
            new_torrent.encoding = std::get<bencode::string>(element->second);
        }
    }

    return new_torrent;
}

std::optional<torrent> from_file(const std::filesystem::path& path) {
    const auto result = bencode::parse(path);
    if (!result.has_value()) {
        return {};
    }

    if (!std::holds_alternative<bencode::dictionary>(result.value())) {
        return {};
    }

    return torrent_from_dictionary(
        std::get<bencode::dictionary>(result.value()));
}
}  // namespace torrent
