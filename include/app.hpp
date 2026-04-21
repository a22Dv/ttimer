#pragma once

#include "utils.hpp"

namespace tmr
{

class Application
{
   public:
    /// Initializes the application, retrieves
    /// configuration files, sets defaults.
    void initialize();

    /// Runs the application main loop.
    /// The method `.initialize()` must be run
    //  before calling `.launch()`.
    void launch();

   private:
    ConsoleMode cmode = {};
};

}  // namespace tmr