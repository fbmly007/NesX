
#if defined(_MSC_VER)
#define SDL_MAIN_HANDLED
#endif

#include "Core/NesXFrame.h"

#ifdef _WIN32
int SDL_UNUSED main(SDL_UNUSED int argc, SDL_UNUSED char *argv[])
#else
int main()
#endif
{
    CNesXFrame frame;
    if (frame.Init())
    {
        frame.LoadGame("./nestest.nes");
        frame.Update();
    }
    frame.Quit();
    return 0;
}
