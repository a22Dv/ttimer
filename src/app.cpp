#include "app.hpp"

#include <chrono>
#include <memory>

#include "cli.hpp"
#include "gui.hpp"
#include "tui.hpp"
#include "ui.hpp"

namespace tmr
{

using namespace std::chrono_literals;

Application::Application(ApplicationState &initstate) : _state{initstate}, _timer{0ns}
{
    if (_state.launch_gui) {
        _ui = std::make_unique<GUI>(*this);
    } else {
        _ui = std::make_unique<TUI>(*this);
    }
}

void Application::launch()
{
    _timer = Timer{_state.timer_duration};
    _ui->launch();  // Control is given to UI (In the case of GTK specifically)
    _ui->quit();
}

bool Application::cycle()
{
    const auto tstate = _timer.cycle();
    switch (tstate) {
        case TimerState::ONGOING: break;
        case TimerState::END:
            if (_state.loop) {
                _timer.restart();
                return true;
            }
            return false;
    }
    return true;
}

bool Application::toggle_pause()
{
    if (!_timer.pause()) {
        _timer.unpause();
        _state.paused = false;
        return false;
    }
    _state.paused = true;
    return true;
}

bool Application::toggle_loop()
{
    _state.loop ^= 1;  // Toggle own arguments.
    return _state.loop;
}

void Application::restart() { _timer.restart(); }

}  // namespace tmr
