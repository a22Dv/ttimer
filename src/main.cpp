
/**
 * TODO:
 * - Implement alarm
 * - Implement GUI
 * - Implement TUI ASCII-art renderer
 */

#ifdef NDEBUG
#include <print>
#endif

#include "app.hpp"
#include "cli.hpp"
#include "gui.hpp"
#include "tui.hpp"

#ifdef NDEBUG
#include "types.hpp"
#endif

using namespace tmr;

int main(int argc, char **argv)
{
    Arguments args = parse_args(argc, argv);

#ifdef NDEBUG
    try {
#endif
        Application app{args};
        app.launch();

#ifdef NDEBUG
    } catch (const std::exception &e) {
        std::print(stderr, "{}", e.what());
        return 1;
    } catch (...) {
        std::print(stderr, ERR_STR("Generic Error."));
        return 1;
    }
#endif
    return 0;
}

