#ifndef __SCENE_H__
#define __SCENE_H__

#include "Vector2.hpp"
#include "Animation.hpp"
#include "Utils.hpp"

#include <vector>
#include <list>
#include <memory>

namespace Sputnik
{
	class Object;
	class Texture;

	// TODO: turn fade into an object
	class Scene
	{
	public:
		class Layer
		{
		public:
			bool follow_camera;
			const std::list<std::shared_ptr<Object>>& get_objects() const { return objects; }
			
			Layer(bool follow_camera = true) : follow_camera(follow_camera) {}

		private:
			std::list<std::shared_ptr<Object>> objects;

			friend Scene;
		};

		static int get_allocated_count();
		
		Scene();
		virtual ~Scene();

		// The scene is initialized for the first time here.
		// App::get_current_scene() is this scene.
		// WARNING: the line above is NOT the case for the scene's constructor!
		virtual void init() = 0;
		
		// This scene is becoming the current scene but was already initialized.
		// (the game left it at some point but then returned to it)
		// App::get_current_scene() is this scene.
		virtual void returned();
		
		// The scene is about to be exited (but maybe not destroyed).
		// The opposite of void returned();
		// App::get_current_scene() is this scene.
		virtual void quit();

		virtual void update(float delta_time);
		virtual void render(float delta_time);

		void add_object(const std::shared_ptr<Object>& obj, int layer);
		// Does NOT free the object when out of scope, useful for
		// objects that are properties of a scene's class
		// (the scene object frees the objects inside it).
		void add_object_unmanaged(Object& obj, int layer);
		void remove_object(const std::shared_ptr<Object>& obj, int layer);
		void remove_object(Object& obj, int layer);

		template<typename T, typename... Args>
		std::shared_ptr<T> create_object(int layer, const Args&... args)
		{
			std::shared_ptr<T> obj = Utils::make_shared<T>(args...);
			add_object(obj, layer);
			return obj;
		}

		// T should accept std::shared_ptr<Object> as parameter
		template<typename T>
		std::shared_ptr<Object> find_object(T func)
		{
			for (Layer& l : layers)
				for (std::shared_ptr<Object>& obj : l.objects)
					if (func(obj))
						return obj;
			return {};
		}

		// Pass ID to add a layer with this specific ID
		void add_layer(int id = -1, Layer params = Layer());
		Layer& get_layer(int layer);

		void set_initialized() { initialized = true; }
		bool is_initialized() const { return initialized; }

	private:
		bool initialized = false;
		std::vector<Layer> layers;
	};

	// TODO: move camera to be a part of the Scene objects
	class Camera
	{
	public:
		Vector2 position = { 0,0 };
		// If the value is greater than 1, zoom in,
		// otherwise zoom out
		float zoom = 1;
		float xmin = 0, xmax = -1;
		float ymin = 0, ymax = -1;
		// Angle is specified in degrees,
		// rotates the camera clockwise
		float angle = 0;

		void apply();

		float get_up() const;
		float get_down() const;
		float get_left() const;
		float get_right() const;
		
		float get_width() const;
		float get_height() const;

		// These two are useful mostly for renderers
		float offset_x() const;
		float offset_y() const;

		void reset();

		// Get current camera
		static Camera& get();
		// Change current camera and return the previous one
		static Camera set(const Camera& camera);
	};

	class BackgroundHandler
	{
	public:
		// TODO: make it private, add speed parameter
		struct Part
		{
			const Texture& texture;
			Rect texture_rect;
			Vector2 pos;
			float xcoef = 1.f, ycoef = 1.f;
			bool xwrap : 1,
				ywrap : 1;

			Part(const Texture& texture)
				: texture(texture), xwrap(false), ywrap(false) {}
		};
		
		virtual ~BackgroundHandler() = default;
		virtual void render(float delta_time);
		void add_part(const Part& p);
		void clear();

	protected:
		std::vector<Part> parts;
	};
}
#endif // __SCENE_H__