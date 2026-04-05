#include "tui.hpp"

#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include <chrono>
#include <print>

#include "app.hpp"
#include "types.hpp"

namespace
{

using namespace tmr;

[[maybe_unused]] std::string tui_str_dhms(int days, int hours, int minutes, int seconds)
{
    if (days) {
        return std::format("| {:02}d {:02}h {:02}m {:02}s |", days, hours, minutes, seconds);
    } else if (hours) {
        return std::format("|     {:02}h {:02}m {:02}s    |", hours, minutes, seconds);
    } else if (minutes) {
        return std::format("|         {:02}m {:02}s       |", minutes, seconds);
    } else { 
        return std::format("|            {:02}s           |", seconds);
    }
}

[[maybe_unused]] std::string tui_str_timestamp(int days, int hours, int minutes, int seconds)
{
    if (days) {
        return std::format("|    {:02}:{:02}:{:02}:{:02}    |", days, hours, minutes, seconds);
    } else if (hours) {
        return std::format("|      {:02}:{:02}:{:02}     |", hours, minutes, seconds);
    } else if (minutes) {
        return std::format("|        {:02}:{:02}      |", minutes, seconds);
    } else {
        return std::format("|         {:02}        |", seconds);
    }
}

char get_input_timeout(chr::milliseconds ms)
{
    struct timeval tv;
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    tv.tv_sec = 0;
    tv.tv_usec = ms.count() * 1000;

    /// NOTE: Changing this to bool results in a sigsegv for some reason.
    int rt = select(1, &fds, NULL, NULL, &tv);
    if (rt > 0) {
        char c;
        read(STDIN_FILENO, &c, 1);
        return c;
    }
    return 0;
}

}  // namespace

namespace tmr
{

using namespace std::chrono_literals;

void TUI::launch()
{
    // Clear console, hide cursor.
    std::print("\e[2J\e[?25l");

    tcgetattr(STDIN_FILENO, &iterm);  // Save config.

    cterm = iterm;
    cfmakeraw(&cterm);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &cterm);

    std::fflush(stdout);

    bool terminate = false;
    while (_app.cycle() && !terminate) {
        update();

        // Blocking until timeout has passed, or user entered input, whichever comes first.
        const char input = get_input_timeout(chr::seconds{1});
        switch (input) {
            case 'q': terminate = true; break;
            default: break;
        }
    }
}

void TUI::update()
{
    std::print("\e[H");
    static winsize w = {};

    winsize wcurrent;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wcurrent) != 0) {
        throw std::runtime_error(ERR_STR("Could not retrieve terminal dimensions."));
    }
    if (w.ws_col != wcurrent.ws_col || w.ws_row != wcurrent.ws_row) {
        std::print("\e[2J");
        std::fflush(stdout);
        w = wcurrent;
    }

    const chr::nanoseconds rdur = _app.timer().remaining_duration();
    const chr::days rday = chr::floor<chr::days>(rdur);
    const chr::hh_mm_ss split{rdur % chr::days{1}};
    const std::string uistr = tui_str_timestamp(rday.count(), split.hours().count(),
                                                split.minutes().count(), split.seconds().count());
    if (uistr.size() < w.ws_col) {
        std::print("\e[{:02};{:02}H", (w.ws_row + 1) / 2, (w.ws_col - uistr.size()) / 2);
    } else {
        std::print("\e[{:02};{:02}H", (w.ws_row + 1) / 2, 1);
    }
    std::print("{}", uistr);
    std::fflush(stdout);
}

void TUI::quit()
{
    // Clear console, set cursor to home, show cursor.
    // NOTE: If process is ever killed unexpectedly this will not run, resulting in a hidden cursor
    // for the user. (e.g. via Ctrl+C).
    std::print("\e[2J\e[H\e[?25h");
    std::fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &iterm);
}

}  // namespace tmr