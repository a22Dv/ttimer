#pragma once

#include "cli.hpp"
#include "timer.hpp"
#include "ui.hpp"

namespace tmr
{

class Application
{
   public:
    Application(Arguments &args);
    void launch();

    /// Runs a single cycle of the timer. Returns a boolean whether this should still be called.
    /// NOTE: Automatically restarts the timer if set to loop.
    bool cycle();

    /**
     * Toggles the pause state of the timer.
     * RETURNS:
     *  - `true` (if was paused)
     *  - `false` (if it was unpaused)
     */
    bool toggle_pause();

    /// Restarts the internal timer.
    void restart();
    const Timer &timer() const noexcept { return _timer; }

   private:
    Arguments _args{};
    std::unique_ptr<UI> _ui = nullptr;
    Timer _timer;
};

}  // namespace tmr