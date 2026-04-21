#pragma once

#include <errno.h>

#include <chrono>
#include <print>
#include <string_view>
#include <system_error>
#include <stdexcept>

namespace tmr
{

/// Message Type.
enum class MType {
    INFO,
    WARN,
    CRIT,
};

inline const char *get_mtype_str(MType type)
{
    const char *ctype = nullptr;
    switch (type) {
        case MType::INFO: ctype = "INFO"; break;
        case MType::WARN: ctype = "WARNING"; break;
        case MType::CRIT: ctype = "CRITICAL"; break;
    }
    return ctype;
}

inline void output_log(MType type, std::string_view str) noexcept
{
    try {
        static const auto czone = std::chrono::current_zone();
        const auto ctime = czone->to_local(std::chrono::system_clock::now());
        const char *ctype = get_mtype_str(type);
        std::print(stderr, "[{:%F %T}, {}] {}\n", ctime, ctype, str);
    } catch (...) {
        // Catch-all to prevent overwriting a
        // possible error with an exception.
    }
}

[[noreturn]] inline void throw_log_runtime(MType type, std::string_view str)
{
    std::string fmtmsg = "";
    try {
        const char *ctype = get_mtype_str(type);
        fmtmsg = std::format("[{}] {}", ctype, str);
    } catch (...) {
        // Catch-all to prevent overwriting a
        // possible error with an exception.
    }
    output_log(type, str);
    throw std::runtime_error(fmtmsg);
}

[[noreturn]] inline void throw_log_system(MType type, std::string_view str)
{
    output_log(type, str);
    throw std::system_error(errno, std::system_category());
}

}  // namespace tmr