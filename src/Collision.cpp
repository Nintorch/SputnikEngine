#include "Collision.hpp"
#include "App.hpp"
#include "Utils.hpp"
#include "Color.hpp"
#include "Renderer.hpp"

#include <algorithm>

namespace Sputnik
{
	namespace Collision
	{
		static constexpr Color debug_color = { 0, 100, 200, 200 };

		bool Group::check_at(Vector2 p)
		{
			for (auto& s : shapes)
				if (s->check_at(p))
					return true;
			return false;
		}

		void Group::add_shape(std::unique_ptr<Shape>&& s)
		{
			shapes.push_back(std::move(s));
		}

		void Group::clear()
		{
			shapes.clear();
		}

		void Group::debug_render(Vector2 offset)
		{
			for (auto& s : shapes)
				s->debug_render(offset);
		}

		bool Rectangle::check_at(Vector2 p)
		{
			if (p.x >= up_left.x && p.x <= up_left.x + size.x &&
				p.y >= up_left.y && p.y <= up_left.y + size.y)
				return true;
			return false;
		}

		void Rectangle::debug_render(Vector2 offset)
		{
			Vector2 pos = up_left + offset;
			get_current_renderer().rectangle_filled({ pos.x, pos.y, size.x, size.y }, debug_color);
		}

		bool Circle::check_at(Vector2 p)
		{
			return p.distance_to(centre) <= radius;
		}

		void Circle::debug_render(Vector2 offset)
		{
			Vector2 pos = centre + offset;
			get_current_renderer().circle_filled(pos, radius, debug_color);
		}

		bool Polygon::check_at(Vector2 p)
		{
			bool inside = false;
			float x = p.x;
			float y = p.y;

			for (size_t i = 0, j = points.size() - 1; i < points.size(); j = i++)
			{
				float xi = points[i].x;
				float yi = points[i].y;
				float xj = points[j].x;
				float yj = points[j].y;

				bool intersect = ((yi > y) != (yj > y))
					&& (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
				if (intersect)
					inside = !inside;
			}

			return inside;
		}

		void Polygon::debug_render(Vector2 offset)
		{
			for (size_t i = 0, j = points.size() - 1; i < points.size(); j = i++)
			{
				Vector2 p1 = points[i] + offset;
				Vector2 p2 = points[j] + offset;
				get_current_renderer().line(p1, p2, debug_color);
			}
		}
	}
}