#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Platform.hpp"
#include "Resource.hpp"
#include "Vector2.hpp"
#include "Color.hpp"
#include "Animation.hpp"

#include <string>
#include <memory>

namespace Sputnik
{
	// Both of them are defined below
	class IWindow;
	class Texture;

	// Defined in Scene.hpp
	class Camera;

	namespace Renderer
	{
		enum class Feature : char
		{
			RENDER_TARGET, // Rendering to textures support
			CAMERA_ZOOM, // Camera zoom support
			CAMERA_ROTATION, // Camera rotation support
			SHADERS, // Custom shaders support
			SCREENSHOT, // Screenshots support

			// Surface should be supported if render targets are supported
			SURFACE = RENDER_TARGET,
		};

		enum class ScaleMode : char
		{
			NEAREST,
			LINEAR,
		};

		enum class BlendMode : char
		{
			NORMAL,
			ADD,
			SUBTRACT,
			MULTIPLY,
		};

		// Base class for renderer-specific texture data
		class TextureData
		{
		public:
			TextureData();
			virtual ~TextureData();
		};

		/*
			IRenderer interface

			Renderer class should handle rendering events as well as handling
			the game window.

			TODO: surface tint
		*/
		class IRenderer
		{
		public:
			virtual ~IRenderer() = default;

			// Don't forget to add "std::unique_ptr<IRenderer> try_setup();" static method to your class

			virtual bool is_feature_supported(Feature feature) = 0;
			virtual bool is_blend_mode_supported(BlendMode mode) = 0;
			
			virtual std::string get_name() = 0;

			virtual void enable_camera(bool flag) = 0;
			virtual void apply_camera(const Camera& camera) = 0;

			virtual void set_default_scale_mode(ScaleMode mode) = 0;
			virtual ScaleMode get_default_scale_mode() = 0;

			virtual IWindow& get_window() = 0;

			/* Rendering */
			
			// Reset render target, start the rendering process
			// and clear the surface
			virtual void start_drawing() = 0;
			// Clear the output screen, draw the surface onto the screen,
			// finish the rendering process and swap the buffers
			virtual void end_drawing() = 0;

			// Get the color that is used to clear the surface on start_drawing()
			virtual Color get_bg_color() = 0;
			virtual void set_bg_color(Color color) = 0;

			// Returns true if successful
			virtual bool set_render_target(Texture& texture) = 0;
			virtual void reset_render_target() = 0;

			virtual Vector2Int get_surface_size() = 0;
			virtual void set_surface_size(int w, int h) = 0;
			// Returns true if successful
			virtual bool use_screen_surface(bool flag) = 0;
			// Use different game windows if available.
			// It would be useful mainly for systems with more than 1 screen
			// (3DS and Wii U homebrew).
			virtual bool use_window(int id) = 0;

			virtual BlendMode get_primitives_blend_mode() = 0;
			virtual void set_primitives_blend_mode(BlendMode mode) = 0;

			virtual void clear(Color color = Colors::NONE) = 0;
			virtual void pixel(Vector2 pos, Color color) = 0;
			virtual void line(Vector2 p1, Vector2 p2, Color color) = 0;
			virtual void rectangle_outline(Rect rect, Color color) = 0;
			virtual void rectangle_filled(Rect rect, Color color) = 0;
			virtual void circle_outline(Vector2 centre, float radius, Color color) = 0;
			virtual void circle_filled(Vector2 centre, float radius, Color color) = 0;

			/*
				All the texture-rendering methods render textures that way
				so the texture's centre is in the specified position
				in playfield coordinates.
				Rotation is specified in degrees and it rotates the texture clockwise.
				Passing negative values in 'scale' vector will flip the drawn texture.
			*/

			virtual void draw_texture(const Texture& texture, Vector2 pos) = 0;
			virtual void draw_texture_scale(const Texture& texture, Vector2 pos, Vector2 scale) = 0;
			virtual void draw_texture_rotate(const Texture& texture, Vector2 pos, float degrees) = 0;
			virtual void draw_texture_part(const Texture& texture, Vector2 pos, Rect texture_rect) = 0;
			virtual void draw_texture_transform(const Texture& texture, Vector2 pos,
				Vector2 scale, float degrees) = 0;
			virtual void draw_texture_transform(const Texture& texture, Vector2 pos,
				Rect texture_rect, Vector2 scale, float degrees) = 0;

			// Save the current rendering target image to a file.
			// Returns true if successful.
			virtual bool take_screenshot(const char* filename) = 0;

		protected:
			/* Next methods are only available to use publicly through Texture class */
			
			virtual std::unique_ptr<TextureData> create_texture(int w, int h, bool texture_target) = 0;
			virtual std::unique_ptr<TextureData> create_texture(const char* filename) = 0;
			// NOTE: the buffer is not freed by this method
			virtual std::unique_ptr<TextureData> create_texture(unsigned char* buffer, int size) = 0;
#if SE_SDL2
			// NOTE: surface is not freed by this method
			virtual std::unique_ptr<TextureData> create_texture(SDL_Surface* surface) = 0;
#endif
			
			virtual int get_texture_width(const Texture& texture) = 0;
            virtual int get_texture_height(const Texture& texture) = 0;
			virtual Vector2Int get_texture_size(const Texture& texture) = 0;

			// Returns true if successful
			virtual void set_texture_scale_mode(Texture& texture, ScaleMode mode) = 0;
			virtual ScaleMode get_texture_scale_mode(const Texture& texture) = 0;

			virtual void set_texture_blend_mode(Texture& texture, BlendMode mode) = 0;
			virtual BlendMode get_texture_blend_mode(const Texture& texture) = 0;

			virtual void set_texture_tint(Texture& texture, Color color) = 0;
			virtual Color get_texture_tint(const Texture& texture) = 0;

			friend Texture;
		};
	}

	/*
		TODO: process window events, remove handle_resolution_update()
	*/
	class IWindow
	{
	public:
		virtual ~IWindow() = default;
		
		virtual std::string get_window_name() = 0;
		virtual void set_window_name(const char* name) = 0;

		virtual Vector2Int get_window_resolution() = 0;
		virtual void set_window_resolution(int w, int h) = 0;
		virtual void handle_resolution_update(int w, int h) = 0;

		virtual Vector2Int get_window_position() = 0;
		virtual void set_window_position(int x, int y) = 0;
		virtual void center_window() = 0;

		virtual void set_resizable(bool enable) = 0;

		virtual void set_fullscreen(bool flag) = 0;
		virtual bool get_fullscreen() = 0;
		
		virtual void toggle_fullscreen()
		{
			set_fullscreen(!get_fullscreen());
		}
	};

	/*
		Easy-to-use wrapper class for renderer-specific texture data.
	*/
	class Texture
	{
	public:
		Texture() = default;
		Texture(Texture&& img);
		Texture(const Texture&) = delete; // No texture copies
		Texture& operator = (Texture&& img);
		Texture& operator = (const Texture&) = delete; // No texture copies

		static int get_allocated_count();

		void create(int w, int h, bool texture_target = false);
		void load_from_file(const char* filename);
		// NOTE: the buffer is not freed by this method
		void load_from_buffer(unsigned char* buffer, int size);
		void load_from_resource(Resource::Handle resource);
#if SE_SDL2
		// NOTE: surface is not freed by this method
		void load_from_SDL_surface(SDL_Surface* surface);
#endif
		
		void free();

		int get_width() const;
		int get_height() const;
		Vector2Int get_size() const;

		Renderer::ScaleMode get_scale_mode() const;
		void set_scale_mode(Renderer::ScaleMode mode);

		Renderer::BlendMode get_blend_mode() const;
		void set_blend_mode(Renderer::BlendMode mode);

		Color get_tint() const;
		void set_tint(Color color);

		template<typename T>
		T& get_data() { return static_cast<T&>(*data); }

		template<typename T>
		const T& get_const_data() const { return static_cast<const T&>(*data); }

	private:
		std::unique_ptr<Renderer::TextureData> data;
	};

	/* Defined in App.cpp */
	IWindow& get_window();
	Renderer::IRenderer& get_current_renderer();
}

#endif // __RENDERER_H__