#include <cstdio>
#include <ctime>
#include <cstdint>
#include <SDL.h>


//constexpr Uint64 kMasterClockSpeed = 236250000 / 11 + 1;
//constexpr Uint64 kClocksPerFrame = kMasterClockSpeed / 60 + 1;

#ifdef _WIN32
int SDL_UNUSED SDL_main(SDL_UNUSED int argc, SDL_UNUSED char *argv[])
#else
int main()
#endif
{
    Uint64 clock_table[] = {89488, 89489, 89488, 89489, 89489};
    Uint64 currentClock = clock_table[0];
    Uint64 clockIndex = 0;


    Uint64 count = 0;
    Uint64 freq = SDL_GetPerformanceFrequency()/60;
    Uint64 totalClocks = 0;



    Uint64 times = 10;

    while(times--)
    {
        totalClocks = 0;
        Uint64 global_start = SDL_GetPerformanceCounter();

        SDL_Event e;
        while(SDL_PollEvent(&e));

        while(SDL_GetPerformanceCounter()-global_start<SDL_GetPerformanceFrequency())
        {
            Uint64 start = SDL_GetPerformanceCounter();
            totalClocks += currentClock;
            for(Uint64 index = 0; index < currentClock; ++index);
            while(SDL_GetPerformanceCounter()-start < freq);
            clockIndex = (clockIndex + 1) % 5;
            currentClock = clock_table[clockIndex];
        }
        printf("total clocks = %llu\n", totalClocks);
    }


//    Uint32 global_start = SDL_GetTicks();
//    Uint32 count = 0;
//    while(SDL_GetTicks()-global_start<1000)
//    {
//        Uint32 start = SDL_GetTicks();
//        count++;
//        while(SDL_GetTicks()-start < 17);
//    }


    return 0;

//    Uint64 clock_table[] = {89488, 89489, 89488, 89489, 89489};
//    Uint64 counter = 0;
//    for(int index = 0; index < 60; ++index)
//    {
//        counter += clock_table[index % 5];
//    }
//
//    printf("counter = %llu\n", counter);
//    printf("real counter = %lf\n", 89488.6 * 60);
//    return 0;
}
