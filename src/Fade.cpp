#include "Fade.hpp"

#include "App.hpp"
#include "Renderer.hpp"
#include "Vector2.hpp"

namespace Sputnik
{
	void Fade::Base::update(float time, float end, Status status) {}
	void Fade::Base::render(float time, float end, Status status)
	{
		get_current_renderer().enable_camera(false);
		
		Vector2Int size = get_current_renderer().get_surface_size();

		if (status == Fade::Status::FADE_IN)
			get_current_renderer().rectangle_filled({ 0, 0, (float)size.x, (float)size.y },
				{ 0,0,0,(unsigned char)(255 - time / end * 255) });
		else
			get_current_renderer().rectangle_filled({ 0, 0, (float)size.x, (float)size.y },
				{ 0,0,0,(unsigned char)(time / end * 255) });

		get_current_renderer().enable_camera(true);
	}

	std::unique_ptr<Fade::Base> Fade::fader;
	float Fade::fade_time;
	float Fade::fade_end;
	Fade::Status Fade::fade_status;
	std::function<void()> Fade::callback;

	Fade::Status Fade::get_status()
	{
		return fade_status;
	}

	float Fade::get_fade_time()
	{
		return fade_time;
	}

	float Fade::get_fade_end()
	{
		return fade_end;
	}

	void Fade::update(float delta_time)
	{
		if (fader)
			fader->update(fade_time, fade_end, fade_status);
		fade_time += delta_time;
		if (fade_time > fade_end) fade_time = fade_end;
	}

	void Fade::render()
	{
		if (fader)
			fader->render(fade_time, fade_end, fade_status);
		if (fader && fade_time >= fade_end)
		{
			fade_status = Status::OFF;
			fader = nullptr;
			if (callback) callback();
		}
	}
}