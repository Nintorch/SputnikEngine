#include "Scene.hpp"
#include "App.hpp"
#include "Utils.hpp"
#include "Object.hpp"
#include "Renderer.hpp"

#include <iostream>
#include <cmath>
#include <tuple>
#include <algorithm>

#ifdef _DEBUG
#	include <typeinfo>
#endif

#pragma warning(push)
#pragma warning(disable: 4244)

namespace Sputnik
{
#ifdef _DEBUG
	static int allocated_count = 0;

	Scene::Scene()
	{
		allocated_count++;
	}
#else
	Scene::Scene() {}
#endif

	Scene::~Scene()
	{
#ifdef _DEBUG
		allocated_count--;
#endif
	}

	int Scene::get_allocated_count()
	{
#ifdef _DEBUG
		return allocated_count;
#else
		return -1;
#endif
	}

	void Scene::returned() {}

	void Scene::quit() {}

	void Scene::update(float delta_time)
	{
		for (Layer& layer : layers)
			for (auto& obj : layer.objects)
				obj->update(delta_time);
	}

	void Scene::render(float delta_time)
	{
		for (Layer& layer : layers)
		{
			get_current_renderer().enable_camera(layer.follow_camera);
			for (auto& obj : layer.objects)
				if (obj->visible)
					obj->render(delta_time);
		}
		
		get_current_renderer().enable_camera(true);
	}

	void Scene::add_object(const std::shared_ptr<Object>& object, int layer)
	{
		if (object->layer >= 0)
		{
			// Save a reference so the object won't get destroyed
			std::shared_ptr<Object> ptr = object;
			get_layer(object->layer).objects.remove(object);
			get_layer(layer).objects.push_back(ptr);

			ptr->layer = layer;
		}
		else
		{
			get_layer(layer).objects.push_back(object);
			object->layer = layer;
		}
		
		Log::info("Object ", object.get(), " (", typeid(*object).name(), ") added to scene ", this,
			" (", typeid(*this).name(), ", layer ", layer, ')');
	}

	void Scene::add_object_unmanaged(Object& obj, int layer)
	{
		add_object(std::shared_ptr<Object>(&obj, [](Object* a) {}), layer);
	}

	void Scene::remove_object(const std::shared_ptr<Object>& obj, int layer)
	{
		auto& objects = get_layer(layer).objects;
		auto it = std::find(objects.begin(), objects.end(), obj);
		if (it != objects.end())
		{
			obj->layer = -1;
			objects.erase(it);
		}
	}

	void Scene::remove_object(Object& obj, int layer)
	{
		auto& objects = get_layer(layer).objects;
		/* Find a std::shared_ptr to the object, and if found,
			delete it from the scene */
		auto it = std::find_if(objects.begin(), objects.end(),
			[&](std::shared_ptr<Object>& o) { return o.get() == &obj; });
		if (it != objects.end())
		{
			obj.layer = -1;
			objects.erase(it);
		}
	}
	
	void Scene::add_layer(int id, Layer params)
	{
		if (id >= 0)
		{
			if (layers.size() <= id)
				layers.resize(id + 1);
			
			layers.at(id) = std::move(params);
		}
		else
			layers.emplace_back(params);
	}

	Scene::Layer& Scene::get_layer(int layer)
	{
		return layers.at(layer);
	}

	void Camera::apply()
	{
		get_current_renderer().apply_camera(*this);
	}

	// TODO: properly handle limit methods if at angle

	float Camera::get_up() const
	{
		int h = get_height();
		float yresult = floor(position.y) - h / 2.0f;
		if (ymin >= 0 && yresult < ymin)
			yresult = ymin;
		if (ymax >= 0 && yresult > (ymax - h / zoom)) yresult = ymax - h / zoom;
		return yresult;
	}

	float Camera::get_down() const
	{
		return get_up() + get_height();
	}

	float Camera::get_left() const
	{
		int w = get_width();
		float xresult = floor(position.x) - w / 2.0f;
		if (xmin >= 0 && xresult < xmin)
			xresult = xmin;
		if (xmax >= 0 && xresult > (xmax - w / zoom)) xresult = xmax - w / zoom;
		return xresult;
	}

	float Camera::get_right() const
	{
		return get_left() + get_width();
	}

	float Camera::get_width() const
	{
		return get_current_renderer().get_surface_size().x / zoom;
	}

	float Camera::get_height() const
	{
		return get_current_renderer().get_surface_size().y / zoom;
	}

	float Camera::offset_x() const
	{
		float w = get_current_renderer().get_surface_size().x / 2.0f;
		return get_left() - w + w / zoom;
	}

	float Camera::offset_y() const
	{
		float h = get_current_renderer().get_surface_size().y / 2.0f;
		return get_up() - h + h / zoom;
	}

	void Camera::reset()
	{
		*this = Camera();
	}

	Camera& Camera::get()
	{
		static Camera camera;
		return camera;
	}

	Camera Camera::set(const Camera& camera)
	{
		Camera ret = get();
		get() = camera;
		return ret;
	}

	void BackgroundHandler::render(float delta_time)
	{
		auto& renderer = get_current_renderer();

		for (Part& p : parts)
		{
			float xcoef = 1.f - p.xcoef;
			float ycoef = 1.f - p.ycoef;
			Vector2 pos = p.pos +
				Vector2{ Camera::get().get_left() * xcoef, Camera::get().get_up() * ycoef };

			// TODO: Y wrap
			if (p.xwrap)
			{
				Vector2 half_size = p.texture_rect.get_size() / 2;
				Vector2 size = p.texture_rect.get_size();

				// Left wrap
				pos.x -= std::ceil((pos.x - Camera::get().get_left()) / size.x + 0.5f) * size.x;

				// Right wrap
				while (pos.x + half_size.x < Camera::get().get_right())
				{
					pos.x += size.x;
					renderer.draw_texture_part(p.texture, pos, p.texture_rect);
				}
			}
			else
				renderer.draw_texture_part(p.texture, pos, p.texture_rect);
		}
	}

	void BackgroundHandler::add_part(const Part& p)
	{
		parts.push_back(p);
	}

	void BackgroundHandler::clear()
	{
		parts.clear();
	}
}
#pragma warning(pop)