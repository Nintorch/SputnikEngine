#ifndef __FADE_H__
#define __FADE_H__

#include "Platform.hpp"
#include "Utils.hpp"

#include <memory>
#include <functional>

namespace Sputnik
{
	class Fade
	{
	public:
		Fade() = delete;

		enum class Status
		{
			FADE_OUT, FADE_IN, OFF
		};

		class Base
		{
		public:
			virtual void update(float time, float end, Status status);
			virtual void render(float time, float end, Status status);
		};

		using Default = Base;

		template<typename T = Default, typename F, typename... Args>
		static void out(float seconds, F c, Args... args)
		{
			if (!fader)
			{
				fader = Utils::make_unique<T>(args...);
				fade_time = 0.0;
				fade_end = seconds;
				fade_status = Status::FADE_OUT;
				callback = c;
			}
		}

		template<typename T = Default>
		static void out(float seconds)
		{
			out<T>(seconds, nullptr);
		}

		template<typename T = Default, typename F, typename... Args>
		static void in(float seconds, F c, Args... args)
		{
			if (!fader)
			{
				fader = Utils::make_unique<T>(args...);
				fade_time = 0.0;
				fade_end = seconds;
				fade_status = Status::FADE_IN;
				callback = c;
			}
		}

		template<typename T = Default>
		static void in(float seconds)
		{
			in<T>(seconds, nullptr);
		}

		static Status get_status();
		static float get_fade_time();
		static float get_fade_end();

		static void update(float delta_time);
		static void render();
		
	private:
		static std::unique_ptr<Base> fader;
		static float fade_time;
		static float fade_end;
		static Status fade_status;
		static std::function<void()> callback;
	};
}
#endif // __FADE_H__