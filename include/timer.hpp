#pragma once

#include <chrono>

namespace tmr
{

namespace chr = std::chrono;

using namespace std::chrono_literals;

using SteadyTime = chr::time_point<chr::steady_clock>;

enum class TimerState { END, ONGOING };

/**
 * NOTE: Timer immediately starts upon object construction.
 * To start at a specified time, call `.restart()`.
 */
class Timer
{
   public:
    Timer(chr::nanoseconds duration) noexcept
        : _tstart{chr::steady_clock::now()},
          _tcurrent{_tstart},
          _tend{_tstart + duration},
          _tduration{duration}
    {
    }

    // Checks the current time and returns the state.
    TimerState cycle() noexcept
    {
        if (_paused) {
            return TimerState::ONGOING;
        }
        _tcurrent = chr::steady_clock::now();
        if (_tcurrent >= _tend) {
            return TimerState::END;
        }
        return TimerState::ONGOING;
    }

    // Resets the timer with the given duration at construction.
    void restart() noexcept
    {
        _tstart = chr::steady_clock::now();
        _tcurrent = _tstart;
        _tend = _tstart + _tduration;
        _tspentpaused = 0ns;
        _paused = false;
    }

    /**
     * Saves the remaining duration and pauses the timer.
     * Returns a boolean indicating the operation's success.
     *
     * NOTE:
     * Calling `pause()` on a _paused timer does not change its
     * state and simply returns `false`.
     */
    bool pause() noexcept
    {
        if (_paused) [[unlikely]] {
            return false;
        }
        _tcurrent = chr::steady_clock::now();
        _paused = true;
        return true;
    }

    /**
     * Unpauses the timer and adjusts its state accordingly.
     * Returns a boolean indicating the operation's success.
     *
     * NOTE:
     * Calling `unpause()` on an un_paused timer does not change its
     * state and simply returns `false`.
     */
    bool unpause() noexcept
    {
        if (!_paused) [[unlikely]] {
            return false;
        }
        const auto ctime = chr::steady_clock::now();
        const auto pause_dur = ctime - _tcurrent;
        _tcurrent = ctime;
        _tend += pause_dur;
        _tspentpaused += pause_dur;
        _paused = false;
        return true;
    }

    // Returns the remaining duration left of the timer.
    chr::nanoseconds remaining_duration() const noexcept
    {
        /// NOTE: `cycle()` should ideally be run before this.
        return std::max(_tend - _tcurrent, chr::nanoseconds{0});
    }

    // Returns the elapsed time of the timer (excluding pauses).
    chr::nanoseconds elapsed() const noexcept
    {
        /// NOTE: `cycle()` should ideally be run before this.
        const auto real_elapsed = _tcurrent - _tstart;
        return real_elapsed - _tspentpaused;
    }

   private:
    SteadyTime _tstart;
    SteadyTime _tcurrent;
    SteadyTime _tend;
    chr::nanoseconds _tduration;

    chr::nanoseconds _tspentpaused = 0ns;
    bool _paused = false;
};

}  // namespace tmr