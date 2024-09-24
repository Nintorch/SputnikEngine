#include "Object.hpp"
#include "App.hpp"
#include "Utils.hpp"
#include "Scene.hpp"

#include <iostream>
#include <algorithm>
#include <limits>

#ifdef _DEBUG
#	include <typeinfo>
#endif

namespace Sputnik
{
#ifdef _DEBUG
	static int allocated_count = 0;
#define ALLOCATED allocated_count++
#define DEALLOCATED allocated_count--
#define GET_ALLOCATED() (allocated_count)
#else
#define ALLOCATED
#define DEALLOCATED
#define GET_ALLOCATED() -1
#endif

	int Object::get_allocated_count()
	{
		return GET_ALLOCATED();
	}

	Object::Object()
	{
		Log::info("Object ", this, " init");
		ALLOCATED;
	}

	Object::~Object()
	{
		Log::info("Object ", this, " freed");
		DEALLOCATED;
	}

	void Object::update(float delta_time)
	{
		velocity += acceleration * App::get_fps_coef();
		position += velocity * delta_time;
	}

	void Object::destroy()
	{
		App::get_current_scene()->remove_object(*this, layer);
	}

	void SpriteObject::update(float delta_time)
	{
		animation.update(delta_time, sprite_rect);
		Object::update(delta_time);
	}

	void SpriteObject::render(float delta_time)
	{
		auto pos = position.floor();

		if (sprite_rect.is_valid())
		{
			get_current_renderer().draw_texture_transform(sprite, pos, sprite_rect,
				scale * Vector2{ flip_x ? -1.f : 1.f, flip_y ? -1.f : 1.f },
				angle);
		}
		else
		{
			get_current_renderer().draw_texture_transform(sprite, pos,
				scale * Vector2{ flip_x ? -1.f : 1.f, flip_y ? -1.f : 1.f },
				angle);
		}
	}

	void TileMapObject::render(float delta_time)
	{
		// TODO: use position of the object
		int left = (int)(Camera::get().get_left() / tile_width);
		int right = (int)(Camera::get().get_right() / tile_width) + 1;
		int top = (int)(Camera::get().get_up() / tile_height);
		int bottom = (int)(Camera::get().get_down() / tile_height) + 1;

		if ((int)Camera::get().angle % 90 != 0)
		{
			int zoom = (int)(1.0f / Camera::get().zoom);
			left -= zoom;
			right += zoom;
			top -= zoom;
			bottom += zoom;
		}

		if (left < 0)
			left = 0;
		if (right > (int)horizontal_tiles)
			right = horizontal_tiles;
		if (top < 0)
			top = 0;
		if (bottom > (int)vertical_tiles)
			bottom = vertical_tiles;

		for (int y = top; y < bottom; y++)
		{
			for (int x = left; x < right; x++)
			{
				uint16_t id = get_tile(x, y);
				// 0 is hardcoded to be transparent
				if (id == 0)
					continue;

				render_tile(id, x, y);
#if SE_DEBUG_COLLISIONS
				get_tile_collision(id).debug_render({ (float)x * tile_width, (float)y * tile_height });
#endif
			}
		}
	}

	void TileMapObject::render_tile(Tile id, int x, int y)
	{
		id--;
		Rect src_rect = get_tile_rect(id);
		Vector2 pos = { (x + 0.5f) * tile_width * 1.f, (y + 0.5f) * tile_height * 1.f };

		get_current_renderer().draw_texture_part(image, pos, src_rect);
	}

	void TileMapObject::load_image(const char* filename)
	{
		image.load_from_file(filename);
		setup();
	}

	void TileMapObject::load_image(unsigned char* buffer, int size)
	{
		image.load_from_buffer(buffer, size);
		setup();
	}

	void TileMapObject::load_image(Resource::Handle resource)
	{
		image.load_from_resource(resource);
		setup();
	}

	// TODO: tool to create layout files
	void TileMapObject::load_layout(unsigned char* data, int size, bool take_ownership, bool copy)
	{
		LayoutHeader* lh = (LayoutHeader*)data;
		data += sizeof(LayoutHeader);
		load_layout(lh->horizontal_tiles, lh->vertical_tiles, (Tile*)data, take_ownership, copy);
	}

	void TileMapObject::load_layout(uint16_t horizontal_tiles, uint16_t vertical_tiles, Tile* data,
				bool take_ownership, bool copy)
	{
		this->horizontal_tiles = horizontal_tiles;
		this->vertical_tiles = vertical_tiles;
		if (take_ownership)
			tile_data = std::shared_ptr<Tile[]>(data);
		else if (copy)
		{
			size_t size = (size_t)horizontal_tiles * vertical_tiles;
			tile_data = Utils::make_shared<Tile[]>(size);
			std::copy(data, data + size, tile_data.get());
		}
		else
			tile_data = std::shared_ptr<Tile[]>(data, [](Tile* p) {});
	}

	Collision::Group& TileMapObject::get_tile_collision(Tile tile)
	{
		return tile_collisions.at(tile - 1);
	}

	uint16_t TileMapObject::get_tile_count() const
	{
		return tilecount_h * tilecount_v;
	}

	bool TileMapObject::check_collision(Vector2 p)
	{
		Vector2 tile_pos = { std::floor(p.x / tile_width), std::floor(p.y / tile_height) };
		Vector2 offset = p - Vector2{tile_pos.x* tile_width, tile_pos.y* tile_height};
		Tile tile = get_tile((int)tile_pos.x, (int)tile_pos.y);
		if (tile)
			return get_tile_collision(tile).check_at(offset);
		return false;
	}

	TileMapObject::Tile TileMapObject::get_tile(int x, int y)
	{
		if (x >= (int)horizontal_tiles || y >= (int)vertical_tiles)
			return 0;
		int pos = y * horizontal_tiles + x;
		return tile_data[pos];
	}

	Rect TileMapObject::get_tile_rect(Tile id)
	{
		int x = id % tilecount_h;
		int y = id / tilecount_h;
		Rect rect = { x * tile_width * 1.f, y * tile_height * 1.f, tile_width * 1.f, tile_height * 1.f };
		return rect;
	}

	void TileMapObject::setup()
	{
		tilecount_h = image.get_width() / tile_width;
		tilecount_v = image.get_height() / tile_height;
		if (tilecount_h * tilecount_v > std::numeric_limits<uint16_t>::max())
		{
			Log::error("TileMapObject: Too many tiles in the tileset. Resetting the tileset.");
			image.create(1, 1);
		}
		tile_collisions.resize(get_tile_count());
	}
}