#pragma once

#include <chrono>
#include <filesystem>

namespace tmr
{

namespace chr = std::chrono;
namespace fs = std::filesystem;

/**
 * Supported arguments:
 *
 * -l / --loop: Loop timer if it has ended,
 */
struct ApplicationState {
    bool loop = false;
    bool launch_gui = true;
    bool paused = false;

    chr::nanoseconds timer_duration;
    chr::sys_time<chr::nanoseconds> application_started;
    fs::path alarm_path;
};

/**
 * Parses the given arguments into an `ApplicationState` struct.
 *
 * NOTE:
 * Internally handles duration finding and interpretation.
 */
ApplicationState parse_args(int argc, const char *const *argv);

}  // namespace tmr