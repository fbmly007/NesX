
#include "Core/NesXFrame.h"

#ifdef _WIN32
int SDL_UNUSED SDL_main(SDL_UNUSED int argc, SDL_UNUSED char *argv[])
#else
int main()
#endif
{
    CNesXFrame frame;
    if (frame.Init())
    {
        frame.LoadGame("Others/nestest.nes");
        frame.Update();
    }
    frame.Quit();
    return 0;
}
