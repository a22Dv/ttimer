#pragma once

#include <chrono>
#include <filesystem>

#include "utilities.hpp"

namespace ttr
{

namespace ch = std::chrono;
namespace fs = std::filesystem;

class Timer
{
   public:
    void start(ch::nanoseconds duration);
    bool toggle_pause();
    ch::nanoseconds get_remaining();
    ch::nanoseconds get_elapsed();

   private:
    ch::time_point<ch::system_clock> _current;
    ch::time_point<ch::system_clock> _end;
    bool _paused;
};

struct RGB {
    u8 r;
    u8 g;
    u8 b;
};

struct ApplicationConfig {
    RGB default_foreground_color;
    RGB default_background_color;
    fs::path default_aaudio_path;
    fs::path default_font_path;
    ch::nanoseconds default_duration;
    std::string formatstr_clock;
    std::string formatstr_timer;
};

struct ApplicationData {
    fs::path data_path;
    fs::path config_path;

    bool supports_truecolor;
};

class Application
{
   public:
    void initialize();
    void launch();

   private:
    Timer _timer;
    ApplicationConfig _config;
    ApplicationData _data;
};

}  // namespace ttr