#include "SDL_GPU.hpp"

#include "Platform.hpp"
#include "App.hpp"
#include "Utils.hpp"
#include "Scene.hpp"

#include "config.hpp"

#if SE_SDL_GPU

#include "SDL_gpu.h"

#if SE_WINDOWS
#include <dwmapi.h>
#endif

namespace Sputnik
{
    namespace Renderer
    {
        /* Texture data */

        SDL_GPU_Renderer::SDL_GPU_TextureData::SDL_GPU_TextureData(
            GPU_Image* image, GPU_FilterEnum filter, GPU_Target* target)
            : image(image), target(target)
        {
            GPU_SetBlending(image, true);
            GPU_SetImageFilter(image, filter);
        }

        SDL_GPU_Renderer::SDL_GPU_TextureData::~SDL_GPU_TextureData()
        {
            GPU_FreeImage(image);
        }

        /* Static methods */

        std::unique_ptr<IRenderer> SDL_GPU_Renderer::try_setup()
        {
            SDL_Window* win = SDL_CreateWindow(
                GAME_NAME,
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                WINDOW_SIZE.x, WINDOW_SIZE.y,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

            if (!win)
            {
                Log::error(SE_FUNCTION, ": Error initializing SDL window: ", SDL_GetError());
                return nullptr;
            }

            GPU_SetInitWindow(SDL_GetWindowID(win));
            GPU_Target* screen = GPU_Init(SURFACE_SIZE.x, SURFACE_SIZE.y, 0);

            if (!screen)
            {
                SDL_DestroyWindow(win);
                GPU_ErrorObject error = GPU_PopErrorCode();
                Log::error(SE_FUNCTION, ": Error initializing SDL GPU renderer: ", error.details, ' ', error.function);
                return nullptr;
            }

            return Utils::make_unique<SDL_GPU_Renderer>(win, screen, SURFACE_SIZE);
        }

        /* Renderer itself */

        SDL_GPU_Renderer::SDL_GPU_Renderer(SDL_Window* win, GPU_Target* screen, Vector2Int surface_size)
            : win(win), screen(screen), surface_size(surface_size), screen_surface(false)
        {
            int w, h;
            SDL_GetWindowSize(win, &w, &h);

            surface = GPU_CreateImage(surface_size.x, surface_size.y, GPU_FORMAT_RGB);
            GPU_SetImageFilter(surface, GPU_FILTER_NEAREST);
            render_target = GPU_LoadTarget(surface);

            GPU_SetShapeBlending(true);

            set_window_resolution(w, h);
        }

        SDL_GPU_Renderer::~SDL_GPU_Renderer()
        {
            GPU_FreeImage(surface);
            GPU_Quit();
            SDL_DestroyWindow(win);
        }

        bool SDL_GPU_Renderer::is_feature_supported(Feature feature)
        {
            switch (feature)
            {
                case Feature::RENDER_TARGET:
                case Feature::CAMERA_ZOOM:
                case Feature::CAMERA_ROTATION:
                case Feature::SCREENSHOT:
                    return true;
                default:
                    return false;
            }
        }

        bool SDL_GPU_Renderer::is_blend_mode_supported(BlendMode mode)
        {
            switch (mode)
            {
                case BlendMode::NORMAL:
                case BlendMode::ADD:
                case BlendMode::SUBTRACT:
                case BlendMode::MULTIPLY:
                    return true;
                default:
                    return false;
            }
        }

        std::string SDL_GPU_Renderer::get_name()
        {
            return Utils::format("SDL gpu ({})", GPU_GetCurrentRenderer()->id.name);
        }

        void SDL_GPU_Renderer::enable_camera(bool flag)
        {
            if (flag != render_target->use_camera)
            {
                GPU_FlushBlitBuffer();
                GPU_EnableCamera(render_target, flag);
            }
        }

        void SDL_GPU_Renderer::apply_camera(const Camera& cam)
        {
            GPU_Target* target = render_target;

            float rad_angle = (cam.angle * M_PI) / 180.0f;
            target->camera.x = (cos(rad_angle) * cam.offset_x() - sin(rad_angle) * cam.offset_y()) * cam.zoom;
            target->camera.y = (sin(rad_angle) * cam.offset_x() + cos(rad_angle) * cam.offset_y()) * cam.zoom;

            target->camera.angle = cam.angle;
            target->camera.zoom_x = cam.zoom;
            target->camera.zoom_y = cam.zoom;
        }

        void SDL_GPU_Renderer::set_default_scale_mode(ScaleMode mode)
        {
            default_scale_mode = mode;
        }

        ScaleMode SDL_GPU_Renderer::get_default_scale_mode()
        {
            return default_scale_mode;
        }

        BlendMode SDL_GPU_Renderer::get_primitives_blend_mode()
        {
            return primitives_blend_mode;
        }

        void SDL_GPU_Renderer::set_primitives_blend_mode(BlendMode mode)
        {
            primitives_blend_mode = mode;
            if (mode == BlendMode::SUBTRACT)
            {
                GPU_SetShapeBlendFunction(GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE,
                    GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE);
                GPU_SetShapeBlendEquation(GPU_EQ_REVERSE_SUBTRACT, GPU_EQ_ADD);
            }
            else
                GPU_SetShapeBlendMode(engine_to_gpu_blend(mode));
        }

        IWindow& SDL_GPU_Renderer::get_window()
        {
            return *this;
        }

        /* Texture-related methods */

        std::unique_ptr<TextureData> SDL_GPU_Renderer::create_texture(int w, int h, bool texture_target)
        {
            GPU_Image* image = GPU_CreateImage(w, h, GPU_FORMAT_RGBA);
            GPU_Target* target = nullptr;

            if (texture_target)
                target = GPU_LoadTarget(image);

            return Utils::make_unique<SDL_GPU_TextureData>(image, engine_to_gpu_filter(default_scale_mode), target);
        }

        std::unique_ptr<TextureData> SDL_GPU_Renderer::create_texture(const char* filename)
        {
            return Utils::make_unique<SDL_GPU_TextureData>(GPU_LoadImage(filename),
                engine_to_gpu_filter(default_scale_mode));
        }

        std::unique_ptr<TextureData> SDL_GPU_Renderer::create_texture(unsigned char* buffer, int size)
        {
            return Utils::make_unique<SDL_GPU_TextureData>(GPU_LoadImage_RW(
                SDL_RWFromConstMem(buffer, size), true), engine_to_gpu_filter(default_scale_mode));
        }

        std::unique_ptr<TextureData> SDL_GPU_Renderer::create_texture(SDL_Surface* surface)
        {
            return Utils::make_unique<SDL_GPU_TextureData>(GPU_CopyImageFromSurface(surface),
                engine_to_gpu_filter(default_scale_mode));
        }

        int SDL_GPU_Renderer::get_texture_width(const Texture& texture)
        {
            return get_image(texture)->w;
        }

        int SDL_GPU_Renderer::get_texture_height(const Texture& texture)
        {
            return get_image(texture)->h;
        }

        Vector2Int SDL_GPU_Renderer::get_texture_size(const Texture& texture)
        {
            GPU_Image* image = get_image(texture);
            return { image->w, image->h };
        }

        void SDL_GPU_Renderer::set_texture_scale_mode(Texture& texture, ScaleMode mode)
        {
            GPU_SetImageFilter(get_image(texture), engine_to_gpu_filter(mode));
        }

        ScaleMode SDL_GPU_Renderer::get_texture_scale_mode(const Texture& texture)
        {
            return gpu_to_engine_filter(get_image(texture)->filter_mode);
        }

        void SDL_GPU_Renderer::set_texture_blend_mode(Texture& texture, BlendMode mode)
        {
            GPU_Image* image = get_image(texture);

            if (mode == BlendMode::SUBTRACT)
            {
                GPU_SetBlendFunction(image, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE,
                    GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE);
                GPU_SetBlendEquation(image, GPU_EQ_REVERSE_SUBTRACT, GPU_EQ_ADD);
            }
            else
                GPU_SetBlendMode(image, engine_to_gpu_blend(mode));

            texture.get_data<SDL_GPU_TextureData>().blend = mode;
        }

        BlendMode SDL_GPU_Renderer::get_texture_blend_mode(const Texture& texture)
        {
            return texture.get_const_data<SDL_GPU_TextureData>().blend;
        }

        void SDL_GPU_Renderer::set_texture_tint(Texture& texture, Color color)
        {
            GPU_SetColor(get_image(texture), color);
        }

        Color SDL_GPU_Renderer::get_texture_tint(const Texture& texture)
        {
            SDL_Color c = get_image(texture)->color;
            return { c.r, c.g, c.b, c.a };
        }

        // TODO: only clear the surface area if using screen surface
        void SDL_GPU_Renderer::clear(Color color)
        {
            GPU_ClearColor(render_target, color);
        }

        void SDL_GPU_Renderer::pixel(Vector2 pos, Color color)
        {
            GPU_Pixel(render_target, pos.x, pos.y, color);
        }

        void SDL_GPU_Renderer::line(Vector2 p1, Vector2 p2, Color color)
        {
            GPU_Line(render_target, p1.x, p1.y, p2.x, p2.y, color);
        }

        void SDL_GPU_Renderer::rectangle_outline(Rect rect, Color color)
        {
            GPU_Rectangle(render_target, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color);
        }

        void SDL_GPU_Renderer::rectangle_filled(Rect rect, Color color)
        {
            GPU_RectangleFilled(render_target, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color);
        }

        void SDL_GPU_Renderer::circle_outline(Vector2 centre, float radius, Color color)
        {
            GPU_Circle(render_target, centre.x, centre.y, radius, color);
        }

        void SDL_GPU_Renderer::circle_filled(Vector2 centre, float radius, Color color)
        {
            GPU_CircleFilled(render_target, centre.x, centre.y, radius, color);
        }

        void SDL_GPU_Renderer::draw_texture(const Texture& texture, Vector2 pos)
        {
            GPU_Blit(get_image(texture), nullptr, render_target, pos.x, pos.y);
        }

        void SDL_GPU_Renderer::draw_texture_scale(const Texture& texture, Vector2 pos, Vector2 scale)
        {
            GPU_BlitScale(get_image(texture), nullptr, render_target, pos.x, pos.y,
                scale.x, scale.y);
        }

        void SDL_GPU_Renderer::draw_texture_rotate(const Texture& texture, Vector2 pos, float degrees)
        {
            GPU_BlitRotate(get_image(texture), nullptr, render_target, pos.x, pos.y, degrees);
        }

        void SDL_GPU_Renderer::draw_texture_part(const Texture& texture, Vector2 pos, Rect texture_rect)
        {
            GPU_Rect rect = texture_rect;
            GPU_Blit(get_image(texture), &rect, render_target, pos.x, pos.y);
        }

        void SDL_GPU_Renderer::draw_texture_transform(const Texture& texture, Vector2 pos,
            Vector2 scale, float degrees)
        {
            GPU_BlitTransform(get_image(texture), nullptr, render_target,
                pos.x, pos.y, degrees, scale.x, scale.y);
        }

        void SDL_GPU_Renderer::draw_texture_transform(const Texture& texture, Vector2 pos,
            Rect texture_rect, Vector2 scale, float degrees)
        {
            GPU_Rect rect = texture_rect;
            GPU_BlitTransform(get_image(texture), &rect, render_target,
                pos.x, pos.y, degrees, scale.x, scale.y);
        }

        bool SDL_GPU_Renderer::take_screenshot(const char* filename)
        {
            GPU_Image* screenshot = GPU_CopyImageFromTarget(render_target);
            bool result = GPU_SaveImage(screenshot, filename, GPU_FILE_AUTO);
            GPU_FreeImage(screenshot);
            return result;
        }

        Color SDL_GPU_Renderer::get_bg_color()
        {
            return bg_color;
        }

        void SDL_GPU_Renderer::set_bg_color(Color color)
        {
            bg_color = color;
        }

        bool SDL_GPU_Renderer::set_render_target(Texture& texture)
        {
            GPU_Target* target = get_target(texture);
            if (target)
            {
                render_target = target;
                return true;
            }
            else
            {
                Log::error(SE_FUNCTION, ": texture is not a target");
                return false;
            }
        }

        void SDL_GPU_Renderer::reset_render_target()
        {
            render_target = screen_surface ? screen : surface->target;
        }

        Vector2Int SDL_GPU_Renderer::get_surface_size()
        {
            return { surface->w, surface->h };
        }

        void SDL_GPU_Renderer::set_surface_size(int w, int h)
        {
            bool reset_target = render_target == surface->target;

            if (surface)
                GPU_FreeImage(surface);

            surface = GPU_CreateImage(w, h, GPU_FORMAT_RGB);
            GPU_SetImageFilter(surface, GPU_FILTER_NEAREST);
            GPU_LoadTarget(surface);
            surface_size = { w, h };

            if (reset_target)
                render_target = surface->target;

            Vector2Int res = get_window_resolution();
            handle_resolution_update(res.x, res.y);
        }

        bool SDL_GPU_Renderer::use_screen_surface(bool flag)
        {
            if (flag)
            {
                GPU_SetVirtualResolution(screen, surface_size.x, surface_size.y);
                GPU_EnableCamera(screen, true);
            }
            else
            {
                GPU_UnsetVirtualResolution(screen);
                GPU_EnableCamera(screen, false);
            }

            screen_surface = flag;
            return true;
        }
        
        bool SDL_GPU_Renderer::use_window(int id)
        {
            // Should several windows be supported? I don't think so.
            return id == 0;
        }

        void SDL_GPU_Renderer::start_drawing()
        {
            reset_render_target();
            if (screen_surface)
            {
                enable_camera(false);
                GPU_Clear(screen);
                GPU_RectangleFilled(screen, 0, 0, surface_size.x, surface_size.y, bg_color);
                enable_camera(true);
            }
            else
                GPU_ClearColor(render_target, bg_color);
        }

        void SDL_GPU_Renderer::end_drawing()
        {
            if (!screen_surface)
            {
                GPU_Clear(screen);
                GPU_BlitRect(surface, nullptr, screen, nullptr);
            }

#if SE_WINDOWS
            DwmFlush();
#endif
            GPU_Flip(screen);
        }

        GPU_Image* SDL_GPU_Renderer::get_image(const Texture& texture)
        {
            return texture.get_const_data<SDL_GPU_TextureData>().image;
        }

        GPU_Target* SDL_GPU_Renderer::get_target(const Texture& texture)
        {
            return texture.get_const_data<SDL_GPU_TextureData>().target;
        }

        GPU_FilterEnum SDL_GPU_Renderer::engine_to_gpu_filter(ScaleMode mode)
        {
            if (mode == ScaleMode::NEAREST)
                return GPU_FILTER_NEAREST;
            else
                return GPU_FILTER_LINEAR;
        }

        ScaleMode SDL_GPU_Renderer::gpu_to_engine_filter(GPU_FilterEnum filter)
        {
            if (filter == GPU_FILTER_NEAREST)
                return ScaleMode::NEAREST;
            else
                return ScaleMode::LINEAR;
        }

        GPU_BlendPresetEnum SDL_GPU_Renderer::engine_to_gpu_blend(BlendMode mode)
        {
            switch (mode)
            {
                default:
                case BlendMode::NORMAL:
                    return GPU_BLEND_NORMAL;
                case BlendMode::ADD:
                    return GPU_BLEND_ADD;
                case BlendMode::MULTIPLY:
                    return GPU_BLEND_MULTIPLY;
            }
        }

        BlendMode SDL_GPU_Renderer::gpu_to_engine_blend(GPU_BlendPresetEnum mode)
        {
            switch (mode)
            {
                default:
                case GPU_BLEND_NORMAL:
                    return BlendMode::NORMAL;
                case GPU_BLEND_ADD:
                    return BlendMode::ADD;
                case GPU_BLEND_MULTIPLY:
                    return BlendMode::MULTIPLY;
            }
        }

        std::string SDL_GPU_Renderer::get_window_name()
        {
            return SDL_GetWindowTitle(win);
        }

        void SDL_GPU_Renderer::set_window_name(const char* name)
        {
            SDL_SetWindowTitle(win, name);
        }

        Vector2Int SDL_GPU_Renderer::get_window_resolution()
        {
            Vector2Int size;
            SDL_GetWindowSize(win, &size.x, &size.y);
            return size;
        }

        void SDL_GPU_Renderer::set_window_resolution(int w, int h)
        {
            handle_resolution_update(w, h);
        }

        void SDL_GPU_Renderer::handle_resolution_update(int w, int h)
        {
            GPU_SetWindowResolution(w, h);

            if (screen_surface)
                GPU_SetVirtualResolution(screen, surface_size.x, surface_size.y);

            Vector2Int surface_size = get_surface_size();
            float coef = std::min((float)w / surface_size.x, (float)h / surface_size.y);
            Vector2 surface_size_float = { surface_size.x * coef, surface_size.y * coef };
            GPU_SetViewport(screen, { w / 2 - surface_size_float.x / 2, h / 2 - surface_size_float.y / 2,
                surface_size_float.x, surface_size_float.y });
        }

        Vector2Int SDL_GPU_Renderer::get_window_position()
        {
            Vector2Int pos;
            SDL_GetWindowPosition(win, &pos.x, &pos.y);
            return pos;
        }

        void SDL_GPU_Renderer::set_window_position(int x, int y)
        {
            SDL_SetWindowPosition(win, x, y);
        }

        void SDL_GPU_Renderer::center_window()
        {
            SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }

        void SDL_GPU_Renderer::set_resizable(bool enable)
        {
            SDL_SetWindowResizable(win, (SDL_bool)enable);
        }

        void SDL_GPU_Renderer::set_fullscreen(bool flag)
        {
            GPU_SetFullscreen(flag, true);
        }

        bool SDL_GPU_Renderer::get_fullscreen()
        {
            return GPU_GetFullscreen();
        }
    }
}
#endif
