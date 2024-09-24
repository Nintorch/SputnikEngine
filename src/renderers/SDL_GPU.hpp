#ifndef __SDL_GPU_H__
#define __SDL_GPU_H__

#include "Renderer.hpp"
#include "Platform.hpp"

#if SE_SDL_GPU

#include "SDL_gpu.h"

namespace Sputnik
{
    namespace Renderer
    {
        class SDL_GPU_Renderer final : public IRenderer, public IWindow
        {
        public:
            class SDL_GPU_TextureData : public TextureData
            {
            public:
                SDL_GPU_TextureData(GPU_Image* image, GPU_FilterEnum filter, GPU_Target* target = nullptr);
                ~SDL_GPU_TextureData() override;

                GPU_Image* image = nullptr;
                GPU_Target* target = nullptr;
                BlendMode blend = BlendMode::NORMAL;
            };

            static std::unique_ptr<IRenderer> try_setup();

            SDL_GPU_Renderer(SDL_Window* win, GPU_Target* screen, Vector2Int surface_size);
            ~SDL_GPU_Renderer();

			bool is_feature_supported(Feature feature) override;
            bool is_blend_mode_supported(BlendMode mode) override;
            
            std::string get_name() override;
            
			void enable_camera(bool flag) override;
            void apply_camera(const Camera& camera) override;
            
            ScaleMode get_default_scale_mode() override;
            void set_default_scale_mode(ScaleMode mode) override;
            
			BlendMode get_primitives_blend_mode() override;
            void set_primitives_blend_mode(BlendMode mode) override;

            IWindow& get_window() override;

            /* Rendering */
            
            Color get_bg_color() override;
            void set_bg_color(Color color) override;
            
            bool set_render_target(Texture& texture) override;
            void reset_render_target() override;
            
            Vector2Int get_surface_size() override;
			void set_surface_size(int w, int h) override;
            bool use_screen_surface(bool flag) override;
            bool use_window(int id) override;

            void clear(Color color) override;
			void pixel(Vector2 pos, Color color);
            void line(Vector2 p1, Vector2 p2, Color color);
            void rectangle_outline(Rect rect, Color color);
            void rectangle_filled(Rect rect, Color color);
			void circle_outline(Vector2 centre, float radius, Color color);
            void circle_filled(Vector2 centre, float radius, Color color);
            
			void draw_texture(const Texture& texture, Vector2 pos) override;
			void draw_texture_scale(const Texture& texture, Vector2 pos, Vector2 scale) override;
			void draw_texture_rotate(const Texture& texture, Vector2 pos, float degrees) override;
		    void draw_texture_part(const Texture& texture, Vector2 pos, Rect texture_rect);
			void draw_texture_transform(const Texture& texture, Vector2 pos,
				Vector2 scale, float degrees) override;
			void draw_texture_transform(const Texture& texture, Vector2 pos,
                Rect texture_rect, Vector2 scale, float degrees) override;
            
            bool take_screenshot(const char* filename) override;

        private:
            SDL_Window* win;
            GPU_Target* screen;
            GPU_Image* surface;
            GPU_Target* render_target; // Current render target
            
            Color bg_color = Colors::BLACK;
            Vector2Int surface_size;
            
            ScaleMode default_scale_mode = ScaleMode::NEAREST;
            BlendMode primitives_blend_mode = BlendMode::NORMAL;
            
            bool screen_surface : 1;

            GPU_Image* get_image(const Texture& texture);
            GPU_Target* get_target(const Texture& texture);

            GPU_FilterEnum engine_to_gpu_filter(ScaleMode mode);
            ScaleMode gpu_to_engine_filter(GPU_FilterEnum filter);

            GPU_BlendPresetEnum engine_to_gpu_blend(BlendMode mode);
            BlendMode gpu_to_engine_blend(GPU_BlendPresetEnum mode);

            /* Texture methods */

            std::unique_ptr<TextureData> create_texture(int w, int h, bool texture_target) override;
            std::unique_ptr<TextureData> create_texture(const char* filename) override;
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
#endif // __SDL_GPU_H__