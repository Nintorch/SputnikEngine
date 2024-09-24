#include "gamemain.hpp"

#include "App.hpp"
#include "Input.hpp"
#include "Resource.hpp"
#include "Utils.hpp"
#include "Language.hpp"

#include <thread>

#include "gamefiles/scenes/Logo.hpp"
#include "gamefiles/scenes/Level.hpp"
#include "gamefiles/InputActions.hpp"

namespace Sputnik
{
	Language::LanguageMap lang_english = {
		{"test_level", L"Welcome to Sputnik Engine test level!"},
		{"by_nintorch", L"Sputnik Engine by Nintorch"},
	};
	
	Language::LanguageMap lang_russian = {
		{"test_level", L"Тестовый уровень движка Sputnik Engine"},
		{"by_nintorch", L"Sputnik Engine сделал Nintorch"},
	};
	
	void game_init()
	{
		Language::new_language("en", L"English", lang_english);
		Language::new_language("ru", L"Русский", lang_russian);
		Language::set_current("en");

		Resource::allow_overwriting(true);
		Resource::load_resource_pack("test.srp");
		get_window().center_window();

		Input::set_key_bind(SDL_SCANCODE_Z, InputActions::JUMP);
		Input::set_key_bind(SDL_SCANCODE_X, InputActions::JUMP);
		Input::set_key_bind(SDL_SCANCODE_C, InputActions::JUMP);

		App::jump_to<Level>();
	}

	void game_update(float delta_time)
	{
		/* pre-update code goes here */

		App::get_current_scene()->update(delta_time);

		/* post-update code goes here */
	}

	void game_render(float delta_time)
	{
		/* pre-render code goes here */

		App::get_current_scene()->render(delta_time);

		/* post-render code goes here */
	}

	void game_quit()
	{
		/* deinitialization code goes here */
	}
}