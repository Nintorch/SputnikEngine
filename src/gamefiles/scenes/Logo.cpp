#include "Logo.hpp"
#include "App.hpp"
#include "Level.hpp"
#include "Fade.hpp"
#include "Utils.hpp"
#include "gamefiles/resources.h"

using namespace Sputnik;

void LogoScene::init()
{
	get_current_renderer().set_bg_color(Colors::BLACK);
	logo.load_from_buffer(res_logopng, res_logopng_size);

	Fade::in(0.5);
}

void LogoScene::returned() {}

void LogoScene::update(float delta_time)
{
	timer += delta_time;

	if (timer > 1.5)
		Fade::out(0.5, [] { App::jump_to<Level>(); });
}

void LogoScene::render(float delta_time)
{
	Vector2Int size = get_current_renderer().get_surface_size();
	get_current_renderer().draw_texture(logo, { size.x / 2.0f, size.y / 2.0f });
}