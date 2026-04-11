#pragma once

#include <cstdint>

namespace tmr {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

enum class TimeType { ABSOLUTE, DURATION, INVALID };

struct RGB {
    u8 r = 0;
    u8 g = 0;
    u8 b = 0;
};

}  // namespace tmr
