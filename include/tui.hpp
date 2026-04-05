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
    void update() override;
    void quit() override;

   private:
    termios cterm{};
    termios iterm{};
};

}  // namespace tmr