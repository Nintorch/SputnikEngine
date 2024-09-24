#include "Platform.hpp"

#if SE_WINDOWS

#include "App.hpp"
#include "Renderer.hpp"
#include "Utils.hpp"
#include "Input.hpp"

#include "SDL.h"
#include "SDL_ttf.h"

// Private code only used in App.cpp
namespace Sputnik::App
{
    void platform_init()
    {
        Log::info("Initializing SDL");
        SDL_Init(SDL_INIT_EVERYTHING);

        if (TTF_Init() < 0)
            Log::error("Error initializing TTF subsystem: ", TTF_GetError());
        else
            Log::info("TTF subsystem initialized");

        Input::set_key_bind(SDL_SCANCODE_UP, Input::UP);
        Input::set_key_bind(SDL_SCANCODE_DOWN, Input::DOWN);
        Input::set_key_bind(SDL_SCANCODE_LEFT, Input::LEFT);
        Input::set_key_bind(SDL_SCANCODE_RIGHT, Input::RIGHT);

        // A default keybind for fullscreen toggle
        Input::set_hotkey(SDL_SCANCODE_F11, [] {
            get_window().toggle_fullscreen();
            });
    }

    extern Uint64 last_time;
    extern SDL_Event event;

    void handle_update()
    {
        last_time = SDL_GetPerformanceCounter();

        while (SDL_PollEvent(&event)) switch (event.type)
        {
            case SDL_QUIT:
                close();
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    Vector2Int res = get_window().get_window_resolution();
                    get_window().handle_resolution_update(res.x, res.y);
                }
                break;

            case SDL_KEYDOWN:
                if (Input::has_key_bind(event.key.keysym.scancode))
                {
                    Input::set_pressed(Input::get_key_bind(event.key.keysym.scancode), true);
                    Input::set_held(Input::get_key_bind(event.key.keysym.scancode), true);
                }

                if (Input::has_hotkey(event.key.keysym.scancode))
                    Input::get_hotkey(event.key.keysym.scancode)();

                Input::set_key_pressed(event.key.keysym.scancode, true);
                Input::set_key_held(event.key.keysym.scancode, true);
                break;

            case SDL_KEYUP:
                if (Input::has_key_bind(event.key.keysym.scancode))
                    Input::set_held(Input::get_key_bind(event.key.keysym.scancode), false);

                Input::set_key_held(event.key.keysym.scancode, false);
                break;

                // TODO
            case SDL_JOYDEVICEADDED:
                break;
            case SDL_JOYDEVICEREMOVED:
                break;

            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button)
                {
                    case 1:
                        Input::Mouse::set_pressed(Input::Mouse::Button::LEFT, true);
                        break;
                    case 2:
                        Input::Mouse::set_pressed(Input::Mouse::Button::MIDDLE, true);
                        break;
                    case 3:
                        Input::Mouse::set_pressed(Input::Mouse::Button::RIGHT, true);
                        break;
                }
                break;
        }
    }
}
#endif