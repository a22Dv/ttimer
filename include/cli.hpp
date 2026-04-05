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
 * -a / --alarm: Alarm if timer has ended. Pass a
 *               path to an audio file otherwise the
 *               default will be used.
 * -ng / --nogui: Don't launch the GUI, goes to a terminal window instead.
 */
struct Arguments {
    bool loop = false;
    bool launch_gui = true;

    chr::nanoseconds timer_duration;
    chr::sys_time<chr::nanoseconds> application_started;
    fs::path alarm_path;
};

/**
 * Parses the given arguments into an `Arguments` struct.
 *
 * NOTE:
 * Internally handles duration finding and interpretation.
 */
Arguments parse_args(int argc, const char *const *argv);

}  // namespace tmr