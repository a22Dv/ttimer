#pragma once

#include <termios.h>

namespace tmr {

class Application;

class TUI {
   public:
    TUI(Application &app) : _app{app} {}
    void launch();
    bool update();
    void quit();

   private:
    bool hide_hotkeys = false;
    termios cterm{};
    termios iterm{};
    Application &_app;
};


}  // namespace tmr