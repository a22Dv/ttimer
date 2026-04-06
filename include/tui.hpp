#pragma once

#include <termios.h>

#include "ui.hpp"

namespace tmr
{

class Application;

class TUI : public UI
{
   public:
    using UI::UI;

    void launch() override;
    bool update() override;
    void quit() override;

   private:
    bool hide_hotkeys = false;
    termios cterm{};
    termios iterm{};
};

}  // namespace tmr