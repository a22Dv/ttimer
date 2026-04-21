#include "app.hpp"

#include <filesystem>
#include <string_view>

namespace
{

using namespace tmr;
namespace fs = std::filesystem;
fs::path get_xdgpath(std::string_view key, const fs::path &tail, const fs::path &fallback)
{
    if (key.empty()) {
        throw_log_runtime(MType::CRIT, "No key specified.");
    }
    const char *envval = std::getenv(key.data());
    if (!envval) {
        return fallback / tail;
    }
    return fs::path(envval) / tail;
}

}  // namespace

namespace tmr
{

namespace fs = std::filesystem;

constexpr std::string_view data_xdgkey = "XDG_DATA_HOME";
constexpr std::string_view config_xdgkey = "XDG_CONFIG_HOME";
constexpr std::string_view cache_xdgkey = "XDG_CACHE_HOME";

static const fs::path home = "~";
static const fs::path data_path = get_xdgpath(data_xdgkey, "ttimer", home / ".local" / "share");
static const fs::path config_path = get_xdgpath(config_xdgkey, "ttimer", home / ".config");

void Application::initialize()
{
    // Get and load available .fgl files.
    // Load configuration, get
}

void Application::launch() {}

}  // namespace tmr