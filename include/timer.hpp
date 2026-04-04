#pragma once

#include <chrono>

namespace tmr
{

namespace chr = std::chrono;

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
        : time_start{chr::steady_clock::now()},
          time_current{time_start},
          time_end{time_start + duration}
    {
    }

    // Checks the current time and returns the state.
    TimerState cycle() noexcept
    {
        if (paused) {
            return TimerState::ONGOING;
        }
        time_current = chr::steady_clock::now();
        if (time_current >= time_end) {
            return TimerState::END;
        }
        return TimerState::ONGOING;
    }

    // Resets the timer with the given duration at construction.
    void restart() noexcept
    {
        const auto dur = time_end - time_start;
        time_start = chr::steady_clock::now();
        time_current = time_start;
        time_end = time_start + dur;
    }

    /**
     * Saves the remaining duration and pauses the timer.
     * Returns a boolean indicating the operation's success.
     *
     * NOTE:
     * Calling `pause()` on a paused timer does not change its
     * state and simply returns `false`.
     */
    bool pause() noexcept
    {
        if (paused) [[unlikely]] {
            return false;
        }
        time_current = chr::steady_clock::now();
        paused = true;
        return true;
    }

    /**
     * Unpauses the timer and adjusts its state accordingly.
     * Returns a boolean indicating the operation's success.
     *
     * NOTE:
     * Calling `unpause()` on an unpaused timer does not change its
     * state and simply returns `false`.
     */
    bool unpause() noexcept
    {
        if (!paused) [[unlikely]] {
            return false;
        }
        const auto ctime = chr::steady_clock::now();
        const auto pause_dur = ctime - time_current;
        time_current = ctime;
        time_end += pause_dur;
        time_spent_paused += pause_dur;
        paused = false;
        return true;
    }

    // Returns the remaining duration left of the timer.
    chr::nanoseconds remaining_duration() const noexcept
    {
        /// NOTE: `cycle()` should ideally be run before this.
        return time_end - time_current;
    }

    // Returns the elapsed time of the timer (excluding pauses).
    chr::nanoseconds elapsed() const noexcept
    {
        /// NOTE: `cycle()` should ideally be run before this.
        const auto real_elapsed = time_current - time_start;
        return real_elapsed - time_spent_paused;
    }

   private:
    SteadyTime time_start;
    SteadyTime time_current;
    SteadyTime time_end;
    
    chr::nanoseconds time_spent_paused;
    bool paused = false;
};

}  // namespace tmr