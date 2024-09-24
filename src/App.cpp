#include <iostream>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <memory>
#include <vector>
#include <functional>

#ifdef _DEBUG
#	include <typeinfo>
#endif

#include "Platform.hpp"

#if SE_SDL2
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_ttf.h"
#endif

#include "App.hpp"
#include "Input.hpp"
#include "Renderer.hpp"
#include "Object.hpp"
#include "Audio.hpp"
#include "Exceptions.hpp"
#include "Scene.hpp"
#include "Utils.hpp"
#include "Fade.hpp"

#include "platform/RenderersAll.hpp"

#include "gamefiles/gamemain.hpp"
#include "gamefiles/config.hpp"

#pragma warning(disable: 4244)

namespace Sputnik
{
	namespace App
	{
		// Every platform should define these functions:

		// platform-specific initialization code
		void platform_init();
		// exception handler (optional, can be empty)
		void setup_exception_handler();
		// called every frame before other engine update code
		void handle_update();
		// a code that runs after all update and render events
		// should include delta_time and fps calculation events
		void post_update(float& delta_time, float& fps);
		// platform-specific deinitialization code
		void platform_quit();

		// Now we define the Sputnik Engine part
		namespace
		{
			std::unique_ptr<Renderer::IRenderer> renderer;
			std::shared_ptr<Scene> scene;
			std::shared_ptr<Scene> next_scene;
			bool running = true;
			float fps = 0;
		}

		void change_scene();

		void init()
		{
			Log::info("Sputnik Engine initialization started!");

			setup_exception_handler();

			Log::info("Platform setup");
			platform_init();

			Log::info("Setting up renderer");

#if SE_SDL_GPU
			CHECK_RENDERER(SDL_GPU_Renderer);
#endif
#if SE_SDL2 && !NO_SDL2_RENDERER
			CHECK_RENDERER(SDL2_Renderer);
#endif

			if (!renderer)
			{
				Log::error("No available renderers");
				throw SputnikException("No available renderers");
			}

			Log::info("Renderer setup: ", get_current_renderer().get_name());

			Log::info("Setting up audio system");
			Audio::init();
			const char* backend_string = Audio::get_engine().getBackendString();
			Log::info("Audio backend: ", (backend_string ? backend_string : "(null)"));

			Log::info("Game-specific initialization code starts");
			game_init();

			if (!next_scene)
			{
				Log::error("You must call App::jump_to() in game_init()!");
				throw SputnikException("No startup scene");
			}

			change_scene();

			Log::info("Sputnik Engine initialized!");
		}

		void quit()
		{
			Log::info("Game specific deinitialization starts");
			game_quit();

			Log::info("Quitting the last scene");
			if (scene)
			{
				scene->quit();
				Log::info("Scene ", scene.get(), " (", typeid(*scene).name(), ") quit");
				scene.reset();
			}

			Log::info("Deinitializing audio system");
			Audio::quit();

#ifdef _DEBUG

#define check_unfreed(name, cls) \
			Log::info("Number of unfreed " name ": ", cls::get_allocated_count()); \
			if (cls::get_allocated_count() > 0) \
				Log::warn("There are unfreed " name "!");

			check_unfreed("textures", Texture);
			check_unfreed("sounds", Audio::Sound::SFX);
			check_unfreed("objects", Object);
			check_unfreed("scenes", Scene);

#endif
			Log::info("Freeing resources");
			Sputnik::Resource::remove_all();

			Log::info("Deinitializing the renderer");
			renderer.reset();

			Log::info("Deinitializing the rest of the engine");
			platform_quit();

			Log::info("Sputnik Engine deinitialized! Goodbye!");
		}

		void update()
		{
			float delta_time = 1 / 60.f;
			fps = 60.f;

			while (running)
			{
				Input::reset_key_pressed();
				Input::reset_action_pressed();
				Input::Mouse::reset();

				handle_update();

				game_update(delta_time);
				Fade::update(delta_time);

				get_current_renderer().start_drawing();

				Camera::get().apply();
				game_render(delta_time);
				Fade::render();

				get_current_renderer().end_drawing();

				if (next_scene)
					change_scene();

				post_update(delta_time, fps);
			}
		}

		void change_scene()
		{
			if (scene)
			{
				scene->quit();
				Log::info("Scene ", scene.get(), " (", typeid(*scene).name(), ") quit");
				scene.reset();
			}

			Camera::get().reset();

			scene = next_scene;
			next_scene = nullptr;

			if (!scene->is_initialized())
			{
				scene->set_initialized();
				scene->init();
			}
			else
			{
				scene->returned();
			}

			Log::info("Scene ", scene.get(), " (", typeid(*scene).name(), ") init");
		}

		void jump_to(const std::shared_ptr<Scene>& s)
		{
			next_scene = s;
		}

		const std::shared_ptr<Scene>& get_current_scene()
		{
			return scene;
		}

		void close()
		{
			running = false;
		}

		bool is_running()
		{
			return running;
		}

		float get_fps()
		{
			return fps;
		}

		float get_fps_coef()
		{
			return 60 / fps;
		}
	}

	Renderer::IRenderer& get_current_renderer()
	{
		return *App::renderer;
	}

	IWindow& get_window()
	{
		return App::renderer->get_window();
	}
}

int main(int argc, char* argv[])
{
	Sputnik::App::init();
	Sputnik::App::update();
	Sputnik::App::quit();
}