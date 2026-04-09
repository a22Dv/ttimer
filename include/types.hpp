#pragma once

#include <cstdint>

namespace tmr {

using u8 = std::uint8_t;

enum class TimeType { ABSOLUTE, DURATION, INVALID };

struct RGB {
    u8 r = 0;
    u8 g = 0;
    u8 b = 0;
};

}  // namespace tmr
