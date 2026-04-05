#include "app.hpp"

#include <chrono>
#include <memory>

#include "gui.hpp"
#include "tui.hpp"
#include "ui.hpp"

namespace tmr
{

using namespace std::chrono_literals;

Application::Application(Arguments &args) : _args{args}, _timer{0ns}
{
    if (_args.launch_gui) {
        _ui = std::make_unique<GUI>(*this);
    } else {
        _ui = std::make_unique<TUI>(*this);
    }
}

void Application::launch()
{
    _timer = Timer{_args.timer_duration};
    _ui->launch();  // Control is given to UI (In the case of GTK specifically)
    _ui->quit();
}

bool Application::cycle()
{
    const auto tstate = _timer.cycle();
    switch (tstate) {
        case TimerState::ONGOING: break;
        case TimerState::END:
            if (_args.loop) {
                _timer.restart();
            }
            return false;
    }
    return true;
}

bool Application::toggle_pause()
{
    if (!_timer.pause()) {
        _timer.unpause();
        return false;
    }
    return true;
}

void Application::restart() { _timer.restart(); }

}  // namespace tmr
