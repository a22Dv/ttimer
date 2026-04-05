#pragma once

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
};

}  // namespace tmr