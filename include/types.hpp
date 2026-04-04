#pragma once

#define ERR_STR(literal_str) "[ERROR]" literal_str
#define STCXPRC static constexpr const

enum class TimeType { ABSOLUTE, DURATION, INVALID };