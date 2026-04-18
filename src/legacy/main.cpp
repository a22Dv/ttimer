#include <clocale>

#ifdef NDEBUG
#include <print>
#endif

#include "legacy/app.hpp"
#include "legacy/cli.hpp"
#include "legacy/tui.hpp"

#ifdef NDEBUG
#include "types.hpp"
#endif

using namespace tmr;

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");
    ApplicationState initstate = parse_args(argc, argv);

#ifdef NDEBUG
    try {
#endif
        Application app{initstate};
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
