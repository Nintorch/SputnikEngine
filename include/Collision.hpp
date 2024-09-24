#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "Vector2.hpp"
#include <vector>
#include <memory>

namespace Sputnik
{
	namespace Collision
	{
		class ICollidable
		{
		public:
			virtual bool check_collision(Vector2 p) = 0;
		};

		class Shape
		{
		public:
			virtual ~Shape() = default;
			virtual bool check_at(Vector2 p) = 0;
			virtual void debug_render(Vector2 offset) = 0;
		};

		class Group
		{
		public:
			bool check_at(Vector2 p);
			void add_shape(std::unique_ptr<Shape>&& s);
			void clear();
			void debug_render(Vector2 offset);
		private:
			std::vector<std::unique_ptr<Shape>> shapes;
		};

		class Rectangle : public Shape
		{
		public:
			Rectangle(Vector2 up_left, Vector2 size)
				: up_left{ up_left }, size{ size } {}
			bool check_at(Vector2 p) override;
			void debug_render(Vector2 offset) override;

			Vector2 up_left, size;
		};

		class Circle : public Shape
		{
		public:
			Circle(Vector2 centre, float radius)
				: centre(centre), radius(radius) {};
			bool check_at(Vector2 p) override;
			void debug_render(Vector2 offset) override;

			Vector2 centre;
			float radius;
		};

		class Polygon : public Shape
		{
		public:
			Polygon(std::vector<Vector2> points)
				: points(points) {}
			bool check_at(Vector2 p) override;
			void debug_render(Vector2 offset) override;

			std::vector<Vector2> points;
		};
	}
}

#endif // __COLLISION_H__