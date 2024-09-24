#ifndef __COLOR_H__
#define __COLOR_H__

#include "Platform.hpp"

#if SE_SDL2
#include "SDL.h"
#endif

namespace Sputnik
{
    struct Color
    {
        uint8_t r = 0,
            g = 0,
            b = 0,
            a = 255;

        constexpr Color() = default;
        constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
            : r(r), g(g), b(b), a(a) {}

        Color operator * (Color c)
        {
            Color ret = {
                (uint8_t)(r * c.r / 255),
                (uint8_t)(g * c.g / 255),
                (uint8_t)(b * c.b / 255),
                (uint8_t)(a * c.a / 255),
            };
            return ret;
        }

#if SE_SDL2
        operator SDL_Color() const { return SDL_Color{ r, g, b, a }; }
#endif
    };

    namespace Colors
    {
        constexpr Color NONE     { 0, 0, 0, 0 };
        constexpr Color BLACK    { 0, 0, 0 };
        constexpr Color GRAY     { 128, 128, 128 };
        constexpr Color WHITE    { 255, 255, 255 };
        constexpr Color RED      { 255, 0, 0 };
        constexpr Color GREEN    { 0, 255, 0 };
        constexpr Color BLUE     { 0, 0, 255 };
        constexpr Color YELLOW   { 255, 255, 0 };
        constexpr Color AQUA     { 0, 255, 255 };
        constexpr Color PURPLE   { 255, 0, 255 };
    }
}

#endif // __COLOR_H__