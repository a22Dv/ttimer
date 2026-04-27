#ifdef NDEBUG
#include <exception>
#include <print>
#endif

#include "utilities.hpp"

class Application;

int main()
{
#ifdef NDEBUG
    try {
#endif
        
#ifdef NDEBUG
    } catch (const std::exception &e) {
        std::print("{}\n", e.what());
    }
#endif
}