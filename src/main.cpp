#ifdef NDEBUG
#include <exception>
#include <print>
#endif

/*
    Feature List:
        - Variable fonts at runtime
        - Variable colors at runtime
        - Variable display format at runtime
        - Smart argument parsing (11:30pm -> N minutes from now)
        - Configurable for:
            - Path to .flf fonts
            - Path to alarm .mp3 file
            - Default duration, color, font
*/

class Application;

int main()
{
#ifdef NDEBUG
    try {
#endif

#ifdef NDEBUG
    } catch (const std::exception &e) {
        std::print("{}", e.what());
    }
#endif
}