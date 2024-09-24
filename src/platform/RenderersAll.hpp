#ifndef __RENDERERSALL_H__
#define __RENDERERSALL_H__

#define CHECK_RENDERER(cls) \
	if (!renderer) \
		renderer = Renderer::cls::try_setup();

/* All possible renderers for all platforms are to be listed here */

#include "renderers/SDL_GPU.hpp"
#include "renderers/SDL2.hpp"

#endif // __RENDERERSALL_H__