#pragma once

#include "cli.hpp"
#include "gui.hpp"
#include "tui.hpp"

namespace
{

using namespace tmr;
struct UI {
    GUI gui;
    TUI tui;
    UI(Application &app) : gui{app}, tui{app} {}
};

}  // namespace

namespace tmr
{

class Application
{
   public:
    Application(Arguments &args);
    void launch();

   private:
    Arguments _args{};
};

}  // namespace tmr