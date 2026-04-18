#include "legacy/cli.hpp"

#include <chrono>
#include <regex>
#include <unordered_map>
#include <utility>

#include "legacy/utils.hpp"

namespace
{

using namespace tmr;
using namespace std::chrono_literals;

namespace chr = std::chrono;

enum class ArgumentType { LOOP, ALARM };
enum class DurationType { FROM_ABSOLUTE, AS_IS };
static const std::unordered_map<std::string, ArgumentType> argtype_map = {
    {"-l", ArgumentType::LOOP},
    {"--loop", ArgumentType::LOOP},
    {"--alarm", ArgumentType::ALARM},
    {"-a", ArgumentType::ALARM},
};

chr::nanoseconds parse_12h_with_unit(std::cmatch &matchobj)
{
    // Matches: ^\s*(0?\d|1[0-2])(:[0-5]\d)?\s*(am|pm)\s*$
    const int h = [&matchobj] {
        int hr = std::stoi(matchobj[1]);
        if ((matchobj[3].str()[0] | 0x20) == 'a') {
            hr = hr == 12 ? 0 : hr;
        } else {
            hr = hr < 12 ? hr + 12 : hr;
        }
        return hr;
    }();
    const int m = matchobj[2].matched ? std::stoi(matchobj[2].str().substr(1)) : 0;
    const auto sysnow = chr::system_clock::now();
    const auto localnow = chr::current_zone()->to_local(sysnow);
    const auto localtarget = [&] {
        auto lt = chr::floor<chr::days>(localnow) + chr::hours{h} + chr::minutes{m};
        if (lt <= localnow) {
            lt += chr::days{1};
        }
        return lt;
    }();
    const auto systarget = chr::current_zone()->to_sys(localtarget, chr::choose::earliest);
    return systarget - sysnow;
}

chr::nanoseconds parse_12h24h_no_unit(std::cmatch &matchobj)
{
    // Matches: ^\s*([0-1]?\d|2[0-3]):([0-5]\d)\s*$
    const int h = std::stoi(matchobj[1].str());
    const int m = std::stoi(matchobj[2].str());

    const auto sysnow = chr::system_clock::now();
    const auto czone = chr::current_zone();
    const auto localnow = czone->to_local(sysnow);
    if (h > 12) {  // 24-hour time.
        auto localtarget = chr::floor<chr::days>(localnow) + chr::hours{h} + chr::minutes{m};
        if (localtarget < localnow) {
            localtarget += chr::days{1};
        }
        const auto systarget = czone->to_sys(localtarget, chr::choose::earliest);
        return systarget - sysnow;
    }
    // 12-hour time.
    auto localtarget_p0 = chr::floor<chr::days>(localnow) + chr::hours{h} + chr::minutes{m};
    auto localtarget_p12 = localtarget_p0 + chr::hours{12};
    if (localtarget_p0 < localnow) {
        localtarget_p0 += chr::days{1};
    }
    if (localtarget_p12 < localnow) {
        localtarget_p12 += chr::days{1};
    }
    const auto systarget_p0 = czone->to_sys(localtarget_p0, chr::choose::earliest);
    const auto systarget_p12 = czone->to_sys(localtarget_p12, chr::choose::earliest);

    return std::min(systarget_p0 - sysnow, systarget_p12 - sysnow);
}

chr::nanoseconds parse_p24h(std::cmatch &matchobj)
{
    // Matches: ^\s*(\d{1,4})\s*$
    const auto tstr = matchobj[1].str();
    if (tstr.size() < 4) {
        return chr::minutes{std::stoi(tstr)};
    }
    const int time = std::stoi(tstr);
    const int as_hours = (time / 1000 % 10) * 10 + (time / 100) % 10;
    const int as_minutes = (time / 10 % 10) * 10 + (time % 10);
    if (as_hours <= 23 && as_minutes <= 59) {
        const auto sysnow = chr::system_clock::now();
        const auto localnow = chr::current_zone()->to_local(sysnow);
        auto lt = chr::floor<chr::days>(localnow) + chr::hours{as_hours} + chr::minutes{as_minutes};
        if (lt < localnow) {
            lt += chr::days{1};
        }
        return chr::current_zone()->to_sys(lt, chr::choose::earliest) - sysnow;
    }
    return chr::minutes{time};
}

chr::nanoseconds parse_direct_dur(std::string_view argument)
{
    // Matches:
    // ^(?:\s*(\d+)\s*((?:days|day|d)|(?:hours?|hrs?|h)|(?:minutes?|mins?|m)|(?:seconds?|secs?|s))\s*)+$

    std::string strarg{argument};
    static const std::regex indivcomp{
        R"R((\d+)\s*((?:days|day|d)|(?:hours?|hrs?|h)|(?:minutes?|mins?|m)|(?:seconds?|secs?|s)))R"};
    static const std::unordered_map<std::string, chr::seconds> hmap = {
        {"days", chr::days{1}},
        {"day", chr::days{1}},
        {"d", chr::days{1}},
        {"hours", chr::hours{1}},
        {"hour", chr::hours{1}},
        {"hrs", chr::hours{1}},
        {"hr", chr::hours{1}},
        {"h", chr::hours{1}},
        {"minutes", chr::minutes{1}},
        {"minute", chr::minutes{1}},
        {"mins", chr::minutes{1}},
        {"min", chr::minutes{1}},
        {"m", chr::minutes{1}},
        {"seconds", chr::seconds{1}},
        {"second", chr::seconds{1}},
        {"secs", chr::seconds{1}},
        {"sec", chr::seconds{1}},
        {"s", chr::seconds{1}},
    };
    std::sregex_iterator cmatch{strarg.begin(), strarg.end(), indivcomp};
    std::sregex_iterator lmatch{};
    chr::nanoseconds duration = 0ns;
    while (cmatch != lmatch) {
        std::smatch cm{*cmatch};
        const long long n = std::stoll(cm[1].str());
        auto ustr = cm[2].str();
        for (auto &c : ustr) {
            c |= 0x20;
        }
        const long long nsec = n * hmap.at(ustr).count();
        duration += chr::seconds{nsec};
        ++cmatch;
    }
    return duration;
}

/**
 * Parses the given argument and interprets it as a duration.
 * Raises an `std::runtime_error` if argument cannot be parsed.
 *
 * Allowed formats:
 * - `<NUMBER>` (Any number, defaults to minutes)
 * - `<NUMBER><d/h/m/s>` Days, hours, minutes, seconds.
 * - `<12-HOUR><AM/PM/am/pm>` (Any 12-hour time (e.g. 12:00PM, 12pm, 10am)
 *    If elapsed, interprets it as tomorrow's time. Note that if AM/PM is
 *    not given, aligns it to match the shortest possible duration on the clock.
 *    (e.g. [11AM, 3] -> 3PM, [1AM, 3] -> 3AM)
 * - `<24-HOUR>` (Any 24-hour time, (e.g. 0500, 0630, 1300))
 *
 * Possible unit variants (case-insensitive unless otherwise specified):
 * - <days/day/d>
 * - <hours/hour/hrs/hr/h>
 * - <minutes/minute/mins/min/m>
 * - <seconds/second/secs/sec/s>
 * - <am/pm>
 *
 * Precedence for shared cases:
 * - <NUMBER>
 *      - As 24-hour time (If explicitly 4-digits and fits 0000 - 2359)
 *      - As minutes
 * - <\d?\d:[0-5]\d>
 *      - Shortest duration 12-hour.
 *      - 24-hour if it falls between 13:00-23:59
 * - <NUMBER><AM/PM>
 *      - Interpret the number hour-first.
 *      - If elapsed, set as tomorrow's time.
 */
std::pair<chr::nanoseconds, DurationType> parse_duration(std::string_view argument)
{
    static const std::regex abstime_12h_24h_nunit{R"R(^\s*([0-1]?\d|2[0-3]):([0-5]\d)\s*$)R"};
    static const std::regex abstime_p24h_nunit{R"R(^\s*(\d{1,4})\s*$)R"};
    static const std::regex abstime_12h_wunit{R"R(^\s*(0?\d|1[0-2])(:[0-5]\d)?\s*(am|pm)\s*$)R",
                                              std::regex_constants::icase};
    static const std::regex direct_duration{
        R"R(^(?:\s*(\d+)\s*((?:days|day|d)|(?:hours?|hrs?|h)|(?:minutes?|mins?|m)|(?:seconds?|secs?|s))\s*)+$)R",
        std::regex_constants::icase};

    std::match_results<std::string_view::const_iterator> matchobj{};
    if (std::regex_match(argument.begin(), argument.end(), matchobj, abstime_12h_wunit)) {
        return {parse_12h_with_unit(matchobj), DurationType::FROM_ABSOLUTE};
    } else if (std::regex_match(
                   argument.begin(), argument.end(), matchobj, abstime_12h_24h_nunit
               )) {
        return {parse_12h24h_no_unit(matchobj), DurationType::FROM_ABSOLUTE};
    } else if (std::regex_match(argument.begin(), argument.end(), matchobj, abstime_p24h_nunit)) {
        return {parse_p24h(matchobj), DurationType::FROM_ABSOLUTE};
    } else if (std::regex_match(argument.begin(), argument.end(), matchobj, direct_duration)) {
        return {parse_direct_dur(argument), DurationType::AS_IS};
    } else {
        throw std::runtime_error(ERR_STR("Unknown argument."));
    }
}

}  // namespace

namespace tmr
{

ApplicationState parse_args(int argc, const char *const *argv)
{
    bool duration_received = false;
    int wait_for_arg = 0;
    ApplicationState args{};

    if (argc < 2) [[unlikely]] {
        throw std::runtime_error(ERR_STR("Insufficient arguments."));
    }

    for (int i = 1; i < argc; ++i) {
        const char *argument = argv[i];
        const auto it = argtype_map.find(argument);
        if (it != argtype_map.end()) {
            switch (it->second) {
                case ArgumentType::LOOP: args.loop = true; continue;
                case ArgumentType::ALARM: wait_for_arg = 1; break;
            }
        }
        switch (wait_for_arg) {
            case 0: break;
            case 1:
                args.alarm_path = argument;
                wait_for_arg = 0;
                continue;
            default: std::unreachable();
        }
        if (duration_received) [[unlikely]] {  // Only one duration term is expected.
            throw std::runtime_error(ERR_STR("Unknown Argument."));
        }

        // Possibly unknown argument or time. Raises an exception if unknown.
        const auto [duration, source_type] = parse_duration(argument);
        if (source_type == DurationType::FROM_ABSOLUTE) {
            args.timer_duration = duration;
            duration_received = true;
        } else {
            args.timer_duration += duration;
        }
    }
    args.application_started = chr::system_clock::now();
    return args;
}

}  // namespace tmr