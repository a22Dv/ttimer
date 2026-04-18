#pragma once

#include "utils.hpp"

namespace tmr
{

class Application
{
   public:
    void initialize();
    void launch();

   private:
    ConsoleMode cmode = {};
};

}  // namespace tmr