#pragma once

#include <wchar.h>

#include <array>
#include <climits>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <ios>
#include <locale>
#include <string>
#include <utility>
#include <vector>

#include "types.hpp"

namespace tmr::fgl
{

namespace fs = std::filesystem;
struct FChar {
    std::vector<std::string> str{};
    int vh = 0;
    int vwidth = 0;
};

using FCharMap = std::array<FChar, UCHAR_MAX>;

struct FigletHeader {
    char hardblank;
    int height;
    int baseline;
    int max_linelength;
    int olayout;
    int comment_lines;
    int print_direction = 0;
    int full_layout = 0;
    int codetag_layout = 0;
};

struct FigletFile {
    FigletHeader header;
    FCharMap map;
};

inline FigletFile open_flf(fs::path path)
{
    std::ifstream file{path};
    if (!file.is_open()) {
        throw std::runtime_error(ERR_STR("Could not open .flf file."));
    }

    FCharMap map = {};

    std::string headerstr{};
    std::getline(file, headerstr);

    FigletHeader hdr{};

    // WARNING: UTF-8 hardblank characters are NOT supported.
    const int scanned = std::sscanf(headerstr.data(), "flf2a%c %d %d %d %d %d %d %d %d",
                                    &hdr.hardblank, &hdr.height, &hdr.baseline, &hdr.max_linelength,
                                    &hdr.olayout, &hdr.comment_lines, &hdr.print_direction,
                                    &hdr.full_layout, &hdr.codetag_layout);
    if (scanned < 6) {
        throw std::runtime_error(ERR_STR("Malformed header string."));
    }
    for (int i = 0; i < hdr.comment_lines; ++i) {
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // NOTE: Does not support extra codetags.
    // Only built to support the standard ASCII set.
    // Rendering also assumes the default, as this is purely a character
    // extractor, there is no kerning support.

    for (char c = ' '; c <= '~'; ++c) {
        FChar character{};

        std::string line{};
        std::wstring wline{};

        // NOTE: Deprecated, but I am not building a UTF-8
        // parser myself just for this single instance.
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter{};
        while (true) {
            line.clear();
            wline.clear();

            std::getline(file, line);
            wline = converter.from_bytes(line);

            int width = 0;
            for (auto &c : wline) {
                if (c == hdr.hardblank) {
                    c = U' ';
                }
                if (c == U'@') {
                    continue;
                }
                width += wcwidth(c);
            }
            int delims = 0;
            for (int i = wline.size() - 1; i >= 0; --i) {
                if (wline[i] == U'@') {
                    wline[i] = U'\0';
                    ++delims;
                }
            }
            line = converter.to_bytes(wline);
            if (delims == 2) {
                break;
            }
            character.str.emplace_back(line.data());
            character.vwidth = width;
        }
        map[c] = std::move(character);
    }

    return {std::move(hdr), std::move(map)};
}

}  // namespace tmr::fgl
