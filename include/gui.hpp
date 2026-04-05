#pragma once

#include "ui.hpp"
namespace tmr
{

class Application;

class GUI : public UI
{
   public:
    using UI::UI;

    void launch() override;
    void update() override;
    void quit() override;
};

}  // namespace tmr