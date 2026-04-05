#pragma once

namespace tmr
{

struct Arguments;
class Application;

class UI
{
   public:
    UI(Application &application) : _app{application} {}
    virtual ~UI() = default;
    virtual void launch() = 0;
    virtual void update() = 0;
    virtual void quit() = 0;

   protected:
    Application &_app;
};

}  // namespace tmr