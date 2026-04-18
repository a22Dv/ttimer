#pragma once

#include <chrono>
#include <print>
#include <string_view>

namespace tmr
{

/// Message Type.
enum class MType {
    INFO,
    WARN,
    CRIT,
};

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