#pragma once

#include "cli.hpp"
#include "timer.hpp"
#include "ui.hpp"

namespace tmr
{

class Application
{
   public:
    Application(ApplicationState &initstate);
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
    bool toggle_loop();

    const ApplicationState &state() const noexcept { return _state; }

    /// Restarts the internal timer.
    void restart();
    const Timer &timer() const noexcept { return _timer; }

    void set_displaymode(int n) noexcept { display_mode = n; }
    int displaymode() const noexcept { return display_mode; }
   private:
    ApplicationState _state{};
    std::unique_ptr<UI> _ui = nullptr;
    int display_mode = 0;
    Timer _timer;
};

}  // namespace tmr