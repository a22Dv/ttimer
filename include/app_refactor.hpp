#pragma once

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <optional>

#include "figlet.hpp"
#include "timer.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace tmr {

namespace ch = std::chrono;
namespace fs = std::filesystem;

static const fs::path appconfig_path =
    create_path_from_vars(".config", "ttimer/config.toml", {"XDG_CONFIG_HOME", "HOME"});

static const fs::path appdata_path =
    create_path_from_vars(".data", "ttimer/data/", {"XDG_DATA_HOME", "HOME"});

struct ApplicationConfig {
    ch::nanoseconds default_timer_duration = 1min;
    fs::path default_alarm = "";
    RGB default_color = {255, 255, 255};
    fs::path default_font = "";
    bool default_loop = false;
    bool default_pause = false;
};

struct ApplicationComponents {
    Timer timer;
};

struct ApplicationData {
    std::vector<fgl::FigletFile> available_fonts = {};
    std::vector<RGB> available_colors = {};
};

struct ApplicationState {
    bool loop = false;
    bool paused = false;
    std::size_t font_loaded = 0;
    ch::nanoseconds timer_duration = 0ns;
    fs::path alarm_loaded = {};
};

struct ApplicationArguments {
    bool loop = false;
    std::optional<ch::nanoseconds> duration = {};
    std::optional<fs::path> alarmpath = {};
    std::optional<fs::path> fontpath = {};
};

class Application {
   public:
    /// Initialize application with arguments given.
    ///
    /// NOTE:
    /// Loads the needed configuration and data files.
    /// Must be called before `run()`.
    void initialize(ApplicationArguments &args);

    /// Launches and enters the application loop.
    void run();

   private:
    ApplicationState _state;
    ApplicationComponents _components;
    ApplicationData _data;
};

}  // namespace tmr