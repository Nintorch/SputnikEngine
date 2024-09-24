#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "Renderer.hpp"
#include "Animation.hpp"
#include "Vector2.hpp"
#include "Collision.hpp"
#include "Font.hpp"
#include "Color.hpp"

namespace Sputnik
{
	/*
		The Object class represents a basic game object with position, velocity, acceleration and visibility.
		It can be subclassed to create more complex objects with custom behavior and rendering.
	*/
	class Object
	{
	public:
		static int get_allocated_count();

		Object();
		virtual ~Object();
		virtual void update(float delta_time);
		virtual void render(float delta_time) = 0;

		// Remove the object from object list of the current scene
		void destroy();
		
		int get_layer() const { return layer; }
		bool exists() const { return layer >= 0; }

		Vector2 position = { 0, 0 };
		Vector2 velocity = { 0, 0 };
		Vector2 acceleration = { 0, 0 };
		bool visible = true;

	private:
		int layer = -1;

		friend class Scene;
	};

	/*
		The SpriteObject class is a subclass of Object that adds rendering functionality for 2D images.
		It can be used to display sprites and animations in the game world.
	*/
	class SpriteObject : public Object
	{
	public:
		SpriteObject()
			: flip_x(false), flip_y(false) {}
		void update(float delta_time) override;
		void render(float delta_time) override;

		const Texture& get_sprite() const { return sprite; }
		
		Vector2 scale = { 1, 1 };
		float angle = 0;
		bool flip_x : 1, flip_y : 1;
		AnimationPlayer animation;

	protected:
		Texture sprite;
		Rect sprite_rect = { 0, 0, -1, -1 };
	};

	/*
		TileMapObject is a subclass of Object that represents a tile map in the game world.
		It contains a tileset, layout and collisions for tiles.
	*/
	class TileMapObject : public Object, public Collision::ICollidable
	{
	public:
		// 65k tiles should be enough
		using Tile = uint16_t;

		struct LayoutHeader
		{
			uint16_t horizontal_tiles;
			uint16_t vertical_tiles;
		};

		TileMapObject(int tile_width, int tile_height)
			: tile_width(tile_width), tile_height(tile_height) {}
		void render(float delta_time) override;
		virtual void render_tile(Tile tile, int x, int y);

		void load_image(const char* filename);
		void load_image(unsigned char* buffer, int size);
		void load_image(Resource::Handle resource);
		Texture& get_image() { return image; }

		void load_layout(unsigned char* data, int size, bool take_ownership = false, bool copy = false);
		void load_layout(uint16_t horizontal_tiles, uint16_t vertical_tiles, Tile* data,
			bool take_ownership = false, bool copy = false);

		Collision::Group& get_tile_collision(Tile tile);
		uint16_t get_tile_count() const;

		bool check_collision(Vector2 p) override;

	protected:
		int tile_width;
		int tile_height;
		// Tile count from the image loaded
		int tilecount_h = 1;
		// Tile count from the image loaded
		int tilecount_v = 1;

		Texture image;

		std::shared_ptr<Tile[]> tile_data;
		uint16_t horizontal_tiles = 0;
		uint16_t vertical_tiles = 0;
		std::vector<Collision::Group> tile_collisions;

		Tile get_tile(int x, int y);
		Rect get_tile_rect(Tile id);
		void setup();
	};

	// Helper functions
	namespace _private
	{
		template<typename T>
		inline Texture _render_text(IFont& font, const T* str, Color color)
		{
			return {};
		}

		template<>
		inline Texture _render_text<char>(IFont& font, const char* str, Color color)
		{
			return font.render_text(str, color);
		}

		template<>
		inline Texture _render_text<wchar_t>(IFont& font, const wchar_t* str, Color color)
		{
			return font.render_unicode(str, color);
		}
	}

	/*
		Text Object is used to display a text in a scene using objects.
		Use TextObject for ASCII text and TextObjectUnicode for wide character strings (i.e. Unicode).
	*/

	template<typename STR>
	class TextObjectBase : public SpriteObject
    {
	public:
		using CHAR = typename STR::value_type;
		
		TextObjectBase() = default;
		TextObjectBase(const std::shared_ptr<IFont>& font, const CHAR* init_str = nullptr)
		{
			use_font(font);
			// Check if the init string exists and not empty
			if (init_str && *init_str != 0)
				set_text(init_str);
		}

		void set_text(const CHAR* str)
		{
			sprite = _private::_render_text<CHAR>(*font, str, color);
			last_text = str;
		}

		const CHAR* get_text() const
		{
			return last_text.c_str();
		}

		void set_color(Color color)
		{
			this->color = color;
		}

		Color get_color() const
		{
			return color;
		}

		void use_font(const std::shared_ptr<IFont>& font)
		{
			this->font = font;
		}

	protected:
		STR last_text;
		std::shared_ptr<IFont> font;
		Color color = Colors::BLACK;
	};

	using TextObject = TextObjectBase<std::string>;
	using TextObjectUnicode = TextObjectBase<std::wstring>;

	// TODO: Particle System
}
#endif // __OBJECT_H__