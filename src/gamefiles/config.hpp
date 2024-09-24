#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "Vector2.hpp"

/* Game config starts here */

#define GAME_NAME       "Sputnik Engine"
#define SURFACE_SIZE    Vector2Int{ 424, 240 }
#define WINDOW_SIZE     (SURFACE_SIZE * 2)

// Audio backend for SoLoud (the enum is defined in soloud.h)
// Check CMakeLists.txt definitions starting with "WITH_" for available backends.
#define AUDIO_BACKEND BACKENDS::AUTO

/* Game config ends here */

#endif // __CONFIG_H__