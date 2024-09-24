#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "engine_config.hpp"

/*
    Platform.hpp
    Sputnik Engine header to adapt the engine for a specific platform
    and perform compile-time platform checking.
*/

// Platform checking

#define SE_WINDOWS _WIN32

// Check if libraries are available

#define SE_SDL2 (SE_WINDOWS)
// SDL gpu is optional
#define SE_SDL_GPU ((SE_WINDOWS /* || more platforms here */ ) && SE_SDL2 && !NO_SDL_GPU)

// Function name

#if defined(_MSC_VER)
#define SE_FUNCTION __FUNCTION__
#elif defined(__GNUC__)
#define SE_FUNCTION __PRETTY_FUNCTION__
#else
#define SE_FUNCTION __func__
#endif

#endif // __PLATFORM_H__