#ifdef NDEBUG
#include <print>
#endif

#include "app.hpp"

using namespace tmr;

int main()
{
#ifdef NDEBUG
    try {
#endif

        Application app = {};
        app.initialize();
        app.launch();

#ifdef NDEBUG
    } catch (const std::exception &e) {
        std::print(stderr, "{}", e.what());
        return -1;
    }
#endif
    return 0;
}