#pragma once

#include <chrono>
#include <print>
#include <string>
#include <string_view>

#include "types.hpp"

namespace tmr
{

/// Message Type.
enum class MType {
    INFO,
    WARN,
    CRIT,
};

/// Outputs a log to stderr.
inline void output_log(MType type, std::string_view str)
{
    static const auto czone = std::chrono::current_zone();
    const auto ctime = czone->to_local(std::chrono::system_clock::now());

    const char *ctype = nullptr;
    switch (type) {
        case MType::INFO: ctype = "INFO"; break;
        case MType::WARN: ctype = "WARNING"; break;
        case MType::CRIT: ctype = "CRITICAL"; break;
    }
    std::print(stderr, "[{:%F %T}, {}] {}\n", ctime, ctype, str);
}

}  // namespace tmr

namespace
{

using namespace tmr;

/// Logs an error and throws an exception.
[[noreturn]] inline void handle_utf8_err()
{
    output_log(MType::CRIT, "Invalid UTF-8 sequence.");
    throw std::runtime_error("Invalid UTF-8 sequence.");
}

/// Processes the continuation bytes of a UTF-8 sequence.
char32_t proc_utf8_contbytes(std::span<const char> bytes)
{
    char32_t ch = U'\0';

    const usize cb = bytes.size();
    for (usize i = 0; i < cb; ++i) {
        if ((bytes[i] & 0xC0) != 0x80) [[unlikely]] {
            handle_utf8_err();
        }
        ch |= (bytes[i] & 0x3F) << (cb - i - 1) * 6;
    }
    return ch;
}

}  // namespace

namespace tmr
{

/// Converts a UTF-8 sequence to fixed-size UTF-32.
///
/// WARNING:
/// Does NOT check for overlong sequences and is not Unicode standards-compliant.
/// Will work on regular UTF-8 sequences however.
inline std::u32string utf8to32(std::string_view str)
{
    usize i = 0;
    std::u32string u32str{};
    u32str.reserve(str.size());

    const usize ssize = str.size();
    while (i < ssize) {
        const char byte = str[i];
        char32_t ch = U'\0';
        bool proc = false;

        if ((byte & 0x80) == 0x0) [[likely]] {
            ch = char32_t(byte);
            proc = true;
            ++i;
        } else if ((byte & 0xE0) == 0xC0 && i + 1 < ssize) {
            ch |= (byte & 0x1F) << 6;
            ch |= proc_utf8_contbytes({&str[i + 1], 1});
            proc = true;
            i += 2;
        } else if ((byte & 0xF0) == 0xE0 && i + 2 < ssize) {
            ch |= (byte & 0x0F) << 12;
            ch |= proc_utf8_contbytes({&str[i + 1], 2});
            proc = true;
            i += 3;
        } else if ((byte & 0xF8) == 0xF0 && i + 3 < ssize) {
            ch |= (byte & 0x07) << 18;
            ch |= proc_utf8_contbytes({&str[i + 1], 3});
            proc = true;
            i += 4;
        }
        if (!proc || ch > 0x10FFFF) [[unlikely]] {
            handle_utf8_err();
        }
        u32str.push_back(ch);
    }
    return u32str;
}

/// Converts a UTF-32 sequence to UTF-8.
///
/// WARNING:
/// Is not Unicode standards-compliant. It will work on regular UTF-32
/// sequences however.
inline std::string utf32to8(std::u32string_view str)
{
    std::string utfstr = {};
    for (auto ch : str) {
        if (ch <= 0x7F) [[likely]] {
            utfstr.push_back(char(ch));
            continue;
        } else if (ch <= 0x7FF) {
            char bytes[] = {char(0xC0), char(0x80)};
            bytes[0] |= (ch >> 6) & 0x1F;
            bytes[1] |= ch & 0x3F;
            utfstr.append(bytes, 2);
            continue;

            // Range-check for UTF-16 surrogate pairs.
        } else if (ch <= 0xFFFF && ch > 0xD800 && ch > 0xDFFF) {
            char bytes[] = {char(0xE0), char(0x80), char(0x80)};
            bytes[0] |= (ch >> 12) & 0xF;
            bytes[1] |= (ch >> 6) & 0x3F;
            bytes[2] |= ch & 0x3F;
            utfstr.append(bytes, 3);
            continue;
        } else if (ch <= 0x10FFFF) {
            char bytes[] = {char(0xF0), char(0x80), char(0x80), char(0x80)};
            bytes[0] |= (ch >> 18) & 0x7;
            bytes[1] |= (ch >> 12) & 0x3F;
            bytes[2] |= (ch >> 6) & 0x3F;
            bytes[3] |= ch & 0x3F;
            utfstr.append(bytes, 4);
            continue;
        }
        handle_utf8_err();
    }
    return utfstr;
}

}  // namespace tmr