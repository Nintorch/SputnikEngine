#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "Platform.hpp"
#include "Vector2.hpp"

#if SE_SDL_GPU
#include "SDL_gpu.h"
#endif

#if SE_SDL2
#include "SDL.h"
#endif

#include <functional>
#include <vector>

namespace Sputnik
{
	struct Rect
	{
		float x, y;
		float w, h;

		bool is_valid()
		{
			return w >= 0 && h >= 0;
		}

		Vector2 get_size()
		{
			return { w, h };
		}

#if SE_SDL_GPU
		operator GPU_Rect() const { return GPU_Rect{ x, y, w, h }; }
#endif

#if SE_SDL2
		operator SDL_FRect() const { return SDL_FRect{ x, y, w, h }; }
		operator SDL_Rect() const { return SDL_Rect{ (int)x, (int)y, (int)w, (int)h }; }
#endif
	};

	struct Animation
	{
		using AnimationFrame = Rect;

		bool loop = false;
		int loop_frame = 0;

		float delay;
		// TODO: case when animation doesn't own the frames array for RAM optimization
		std::vector<AnimationFrame> frames;

		Animation(float delay, std::vector<AnimationFrame> frames);
		Animation(float delay, int loop_frame, std::vector<AnimationFrame> frames);
	};

	class AnimationPlayer
	{
	public:
		float timer = 0;
		int frame = 0;

		/*
			Play the animation.
			The animation object has to exist while the AnimationPlayer is playing it.
		*/
		void play(const Animation& animation);
		bool update(float delta_time, Rect& sprite);
		// Start the current animation from the start
		void reset();
		void stop();

		bool is_playing(const Animation& animation);
		
		// Set a callback when the current animation ends.
		// This does not include changing the current animation.
		void set_finish_callback(std::function<void()> callback);
		void reset_finish_callback();

	private:
		const Animation* animation = nullptr;
		std::function<void()> callback;
	};
}
#endif // __ANIMATION_H__