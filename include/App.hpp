#ifndef __APP_H__
#define __APP_H__

#include "Platform.hpp"
#include "Utils.hpp"

namespace Sputnik
{
	class Scene;

	namespace App
	{
		// Jump to a scene.
		// App::get_current_scene() is not yet the scene passed here,
		// it will be on the next frame.
		void jump_to(const std::shared_ptr<Scene>& scene);

		// Jump to a scene using newly created std::shared_ptr
		// (arguments to this method are passed to the scene's constructor)
		template<typename T, typename... Args>
		std::shared_ptr<T> jump_to(Args&... a)
		{
			std::shared_ptr<T> ptr = Utils::make_shared<T>(a...);
			jump_to(ptr);
			return ptr;
		}

		const std::shared_ptr<Scene>& get_current_scene();
		void close();
		bool is_running();

		float get_fps();
		float get_fps_coef();
	}
}
#endif // __APP_H__