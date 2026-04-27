#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace ttr
{

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using usize = std::size_t;
using isize = std::ptrdiff_t;
using f32 = float;
using f64 = double;

enum class ErrorType : u8 {
    CRITICAL,
    WARNING,
    INFORMATION,
};

inline std::string logformat(ErrorType errtype, std::string_view msg)
{
    std::string_view prfx;
    switch (errtype) {
        case ErrorType::CRITICAL: prfx = "CRITICAL"; break;
        case ErrorType::WARNING: prfx = "WARNING"; break;
        case ErrorType::INFORMATION: prfx = "INFORMATION"; break;
    }
    const auto ctime = std::chrono::system_clock::now();
    const auto fmsg = std::format("[{} {}] {}", prfx, ctime, msg);
    return fmsg;
}

[[noreturn]]
inline void throw_runtime(ErrorType errtype, std::string_view msg)
{
    throw std::runtime_error(logformat(errtype, msg));
}

[[noreturn]]
inline void throw_system(ErrorType errtype, std::string_view msg)
{
    const int errnov = errno;
    auto fmtmsg = logformat(errtype, msg);
    fmtmsg.push_back(' ');  // Make space before system_err appends sysmsg.
    throw std::system_error(errnov, std::system_category(), fmtmsg);
}

}  // namespace ttr