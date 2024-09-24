#include "Platform.hpp"

#if SE_SDL2

#include <exception>

#include "Utils.hpp"

#include "SDL.h"
#include "SDL_ttf.h"

namespace Sputnik::App
{
    SDL_Event event;
    Uint64 now_time = 0, last_time;

    void setup_exception_handler()
    {
        std::set_terminate([] {
            try
            {
                if (std::current_exception())
                    std::rethrow_exception(std::current_exception());
            }
            catch (std::exception& e)
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                    "Sputnik Engine crashed", e.what(), nullptr);
            }
            catch (...)
            {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                    "Sputnik Engine crashed",
                    "Exception of unknown type was thrown", nullptr);
            }
            });
    }

    void post_update(float& delta_time, float& fps)
    {
        now_time = SDL_GetPerformanceCounter();
        delta_time = (float)(now_time - last_time) / SDL_GetPerformanceFrequency();
        fps = 1 / delta_time;
    }

    void platform_quit()
    {
        Log::info("Deinitializing TTF subsystem");
        TTF_Quit();

        Log::info("Quitting SDL");
        SDL_Quit();
    }
}
#endif