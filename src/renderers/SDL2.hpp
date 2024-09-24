#ifndef __SDL2_H__
#define __SDL2_H__

#include "Renderer.hpp"
#include "Platform.hpp"

#if SE_SDL2 && !NO_SDL2_RENDERER

#include "SDL.h"

namespace Sputnik
{
    namespace Renderer
    {
        class SDL2_Renderer final : public IRenderer, public IWindow
        {
        public:
            class SDL2_TextureData : public TextureData
            {
            public:
                SDL2_TextureData(SDL_Texture* texture, SDL_ScaleMode mode);
                ~SDL2_TextureData() override;

                SDL_Texture* texture;
                Vector2Int size;
            };
            
            static std::unique_ptr<IRenderer> try_setup();

            SDL2_Renderer(SDL_Window* win, SDL_Renderer* render);
            ~SDL2_Renderer();

			bool is_feature_supported(Feature feature) override;
			bool is_blend_mode_supported(BlendMode mode) override;
			
			std::string get_name() override;

			void enable_camera(bool flag) override;
			void apply_camera(const Camera& camera) override;
			
			ScaleMode get_default_scale_mode() override;
			void set_default_scale_mode(ScaleMode mode) override;
			
            IWindow& get_window() override;

			/* Rendering */

			// Get the color that is used to clear the surface on start_drawing()
			Color get_bg_color() override;
			void set_bg_color(Color color) override;

			// Returns true if successful
			bool set_render_target(Texture& texture) override;
			void reset_render_target() override;
			
			Vector2Int get_surface_size() override;
			void set_surface_size(int w, int h) override;
			// Returns true if successful
			bool use_screen_surface(bool flag) override;
            bool use_window(int id) override;
			
			BlendMode get_primitives_blend_mode() override;
			void set_primitives_blend_mode(BlendMode mode) override;

			void clear(Color color = Colors::NONE) override;
			void pixel(Vector2 pos, Color color) override;
			void line(Vector2 p1, Vector2 p2, Color color) override;
			void rectangle_outline(Rect rect, Color color) override;
			void rectangle_filled(Rect rect, Color color) override;
			void circle_outline(Vector2 centre, float radius, Color color) override;
			void circle_filled(Vector2 centre, float radius, Color color) override;

			/* All the texture-rendering methods render textures that way
				so the texture's centre is in the specified position
				in playfield coordinates */

			void draw_texture(const Texture& texture, Vector2 pos) override;
			void draw_texture_scale(const Texture& texture, Vector2 pos, Vector2 scale) override;
			void draw_texture_rotate(const Texture& texture, Vector2 pos, float degrees) override;
			void draw_texture_part(const Texture& texture, Vector2 pos, Rect texture_rect) override;
			void draw_texture_transform(const Texture& texture, Vector2 pos,
				Vector2 scale, float degrees) override;
			void draw_texture_transform(const Texture& texture, Vector2 pos,
				Rect texture_rect, Vector2 scale, float degrees) override;
			
			bool take_screenshot(const char* filename) override;

        private:
            SDL_Window* win;
			SDL_Renderer* render;
			SDL_Texture* surface;
			
			Color bg_color = Colors::BLACK;
			Vector2Int surface_size;
			ScaleMode default_scale_mode = ScaleMode::NEAREST;
			SDL_BlendMode primitives_blend_mode = SDL_BLENDMODE_BLEND;
			SDL_BlendMode subtract_blend_mode;
			
			Vector2 camera_pos = {};
			float camera_angle = 0; // degrees
			float camera_zoom = 1;

            Uint64 frame_start;
			
			bool use_software_vsync : 1,
				screen_surface : 1,
				camera_enabled : 1,
				camera_zoom_or_angle : 1;

            SDL_Texture* get_texture(const Texture& texture);
            // Position, rotate and scale the vector according to camera and the texture size
			void camera_vector(Vector2& pos, Vector2& size, float& degrees);

			SDL_ScaleMode engine_to_sdl_scale(ScaleMode mode);
			ScaleMode sdl_to_engine_scale(SDL_ScaleMode mode);
			
			SDL_BlendMode engine_to_sdl_blend(BlendMode mode);
			BlendMode sdl_to_engine_blend(SDL_BlendMode mode);

			/* Texture methods */

            std::unique_ptr<TextureData> create_texture(int w, int h, bool texture_target) override;
            std::unique_ptr<TextureData> create_texture(const char* filename) override;
            // Data at buffer can be in any format, but BMP and PNG should always be supported
            std::unique_ptr<TextureData> create_texture(unsigned char* buffer, int size) override;
			std::unique_ptr<TextureData> create_texture(SDL_Surface* surface) override;
			
			int get_texture_width(const Texture& texture) override;
            int get_texture_height(const Texture& texture) override;
			Vector2Int get_texture_size(const Texture& texture) override;
			
			void set_texture_scale_mode(Texture& texture, ScaleMode mode) override;
			ScaleMode get_texture_scale_mode(const Texture& texture) override;
			
			void set_texture_blend_mode(Texture& texture, BlendMode mode) override;
			BlendMode get_texture_blend_mode(const Texture& texture) override;
			
			void set_texture_tint(Texture& texture, Color color) override;
            Color get_texture_tint(const Texture& texture) override;

			void start_drawing() override;
			void end_drawing() override;

			/* Window methods */

			std::string get_window_name() override;
			void set_window_name(const char* name) override;
		
			Vector2Int get_window_resolution() override;
            void set_window_resolution(int w, int h) override;
            void handle_resolution_update(int w, int h) override;

			Vector2Int get_window_position() override;
			void set_window_position(int x, int y) override;
			void center_window() override;
			
			void set_resizable(bool enable) override;

			void set_fullscreen(bool flag) override;
			bool get_fullscreen() override;
		};
    }
}

#endif

#endif // __SDL2_H__