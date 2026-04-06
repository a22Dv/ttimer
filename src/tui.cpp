#include "tui.hpp"

#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include <chrono>
#include <cmath>
#include <format>
#include <print>
#include <tuple>
#include <utility>

#include "app.hpp"
#include "figlet.hpp"
#include "types.hpp"

namespace
{

using namespace tmr;
using namespace std::chrono_literals;

int reverse_digits(int n)
{
    int rn = 0;
    while (n) {
        const int d = n % 10;
        rn *= 10;
        rn += d;
        n /= 10;
    }
    return rn;
}

int count_digits(int n)
{
    if (n == 0) {
        return 1;
    }
    return std::log10(n) + 1;
}

void format_number(const fgl::FCharMap &fmap, std::vector<fgl::FChar> &v, int n, int min_dwidth)
{
    int rn = reverse_digits(n);
    const int digits = count_digits(n);

    if (digits < min_dwidth) {
        for (int i = digits; i <= min_dwidth - digits; ++i) {
            v.push_back(fmap['0']);
        }
    }
    for (int i = 0; i < digits; ++i) {
        const int d = rn % 10;
        v.push_back(fmap['0' + d]);
        rn /= 10;
    }
}

[[maybe_unused]] std::tuple<std::vector<std::string>, int, int, int> display_timestamp_ver(
    const fgl::FigletFile &ffile, int days, int hours, int minutes, int seconds)
{
    const fgl::FCharMap &fmap = ffile.map;
    std::vector<fgl::FChar> display = {};
    // Display string: <days>:<hours>:<minutes>:<seconds>
    if (days > 0) {
        format_number(fmap, display, days, 2);
        display.push_back(fmap[':']);
    }
    format_number(fmap, display, hours, 2);
    display.push_back(fmap[':']);
    format_number(fmap, display, minutes, 2);
    display.push_back(fmap[':']);
    format_number(fmap, display, seconds, 2);

    const int th = display.front().str.size();
    int tw = 0;
    for (auto &ch : display) {
        tw += ch.vwidth;
    }
    std::vector<std::string> dstrs{};
    for (auto ch : display) {
        for (auto line : ch.str) {
            dstrs.push_back(line);
        }
    }
    // String lines of the characters, total height, total width, number of characters.
    return {dstrs, th, tw, display.size()};
}

[[maybe_unused]] std::tuple<std::vector<std::string>, int, int, int> display_labeled_ver(
    const fgl::FigletFile &ffile, int days, int hours, int minutes, int seconds)
{
    // Hardcoded (test).
    const fgl::FCharMap &fmap = ffile.map;

    std::vector<fgl::FChar> display = {};
    // Display string: <days> d <hrs> h <mins> m <seconds> s

    if (days > 0) {
        format_number(fmap, display, days, 0);
        for (auto c : std::string_view{"d "}) {
            display.push_back(fmap[c]);
        }
    }
    if (hours > 0) {
        format_number(fmap, display, hours, 0);
        for (auto c : std::string_view{"h "}) {
            display.push_back(fmap[c]);
        }
    }
    if (minutes > 0) {
        format_number(fmap, display, minutes, 0);
        for (auto c : std::string_view{"m "}) {
            display.push_back(fmap[c]);
        }
    }
    format_number(fmap, display, seconds, 0);
    for (auto c : std::string_view{"s"}) {
        display.push_back(fmap[c]);
    }

    const int th = display.front().str.size();
    int tw = 0;
    for (auto &ch : display) {
        tw += ch.vwidth;
    }
    std::vector<std::string> dstrs{};
    for (auto ch : display) {
        for (auto line : ch.str) {
            dstrs.push_back(line);
        }
    }
    // String lines of the characters, total height, total width, number of characters.
    return {dstrs, th, tw, display.size()};
}

// String, Visual Height, Visual Width, Character Count
using DisplayOutput = std::tuple<std::vector<std::string>, int, int, int>;
DisplayOutput get_displaystr(const ApplicationState &state, chr::days days, chr::hours hours,
                             chr::minutes minutes, chr::seconds seconds,
                             chr::nanoseconds subseconds)
{
    static const fgl::FigletFile ffile = fgl::open_flf("./data/ANSI Shadow.flf");

    const int dc = days.count();
    const int hc = hours.count();
    const int mc = minutes.count();
    const int sc = seconds.count();

    auto [dstr, vh, vw, cc] = [&] {
        const int mode = 1;  // Hardcoded (test)
        switch (mode) {
            case 0: return display_labeled_ver(ffile, dc, hc, mc, sc);
            case 1: return display_timestamp_ver(ffile, dc, hc, mc, sc);
            default: std::unreachable();
        }
    }();

    const auto tduration = days + hours + minutes + seconds + subseconds;
    const chr::seconds duration = chr::round<chr::seconds>(tduration);
    const auto lctime = chr::current_zone()->to_local(chr::system_clock::now() + duration);
    const auto lctime_r = chr::round<chr::seconds>(lctime);

    dstr.push_back(std::format("{:^{}}", "", vw));
    dstr.push_back(std::format("{:^{}}", std::format(" -- {} -- ", lctime_r), vw));
    dstr.push_back(std::format("{:^{}}", "", vw));

    const char *paused_choice = state.paused ? "Play" : "Pause";
    const char *loop_choice = state.loop ? "Unloop" : "Loop";
    const std::string substr = std::format("[P] {} [L] {} [Q] Quit", paused_choice, loop_choice);
    dstr.push_back(std::format("{:^{}}", substr, vw));
    return {dstr, vh, vw, cc};
}

char get_input_timeout(chr::microseconds ms)
{
    struct timeval tv;
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    tv.tv_sec = 0;
    tv.tv_usec = ms.count();

    // NOTE: Changing this to bool results in a SIGSEGV for some reason.
    const int rt = select(1, &fds, NULL, NULL, &tv);
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
    auto next_tick = chr::steady_clock::now() + chr::seconds{1};
    while (_app.cycle() && !terminate) {
        update();

        // Blocking until timeout has passed, or user entered input, whichever comes first.
        const auto now = chr::steady_clock::now();
        const auto timeout = next_tick < now ? 0ns : next_tick - now;
        const char input = get_input_timeout(chr::floor<chr::microseconds>(timeout));
        const auto timeout_duration = chr::steady_clock::now() - now;
        if (timeout_duration >= timeout) {
            next_tick += chr::seconds{1};
        }
        switch (input) {
            case 'q': terminate = true; break;
            case 'p': _app.toggle_pause(); break;
            case 'l': _app.toggle_loop(); break;
            case 0x03: terminate = true; break;  // Ctrl+C SIGINT
            default: break;
        }
    }
}

void TUI::update()
{
    std::print("\e[H");
    static winsize w = {};

    winsize wcurrent = {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wcurrent) != 0) {
        throw std::runtime_error(ERR_STR("Could not retrieve terminal dimensions."));
    }
    if (w.ws_col != wcurrent.ws_col || w.ws_row != wcurrent.ws_row) {
        std::print("\e[2J");
        std::fflush(stdout);
        w = wcurrent;
    }

    _app.cycle();
    const chr::nanoseconds rdur = _app.timer().remaining_duration();
    const chr::days rday = chr::floor<chr::days>(rdur);
    const chr::hh_mm_ss split{rdur % chr::days{1}};

    const auto hours = split.hours();
    const auto minutes = split.minutes();
    const auto seconds = split.seconds();
    const auto subseconds = split.subseconds();
    const auto [dstr, vh, vw, cc] =
        get_displaystr(_app.state(), rday, hours, minutes, seconds, subseconds);

    if (w.ws_col < vw || w.ws_row < vh) {
        return;  // Do not display if cut off.
    }
    const int xpos = (w.ws_col - vw + 1) / 2;
    const int ypos = (w.ws_row - vh + 1) / 2;

    for (int i = 0; i < vh; ++i) {
        std::string row{};
        for (int j = 0; j < cc; ++j) {
            row.append(dstr[j * vh + i]);
        }
        std::print("\e[{};{}H\e[2K", ypos + i, xpos);
        std::print("{}", row);
    }
    for (int i = 0; i < 4; ++i) {  // Subtext.
        std::print("\e[{};{}H\e[2K", ypos + vh + i, xpos);
        std::print("{}", dstr[cc * vh + i]);
    }
    std::fflush(stdout);
}

void TUI::quit()
{
    // Clear console, set cursor to home, show cursor.
    // NOTE: If process is ever killed unexpectedly this will not run, resulting in a hidden cursor
    // for the user. (e.g. kill).
    std::print("\e[2J\e[H\e[?25h");
    std::fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &iterm);
}

}  // namespace tmr