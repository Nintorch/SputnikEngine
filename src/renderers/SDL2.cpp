#include "SDL2.hpp"

#include "Platform.hpp"
#include "Scene.hpp"
#include "Utils.hpp"
#include "config.hpp"

#if SE_SDL2 && !NO_SDL2_RENDERER

#include "SDL_image.h"

namespace Sputnik
{
    namespace Renderer
    {
        SDL2_Renderer::SDL2_TextureData::SDL2_TextureData(SDL_Texture* texture, SDL_ScaleMode mode)
            : texture(texture)
        {
            SDL_QueryTexture(texture, nullptr, nullptr, &size.x, &size.y);
            SDL_SetTextureScaleMode(texture, mode);
        }

        SDL2_Renderer::SDL2_TextureData::~SDL2_TextureData()
        {
            SDL_DestroyTexture(texture);
        }

        std::unique_ptr<IRenderer> SDL2_Renderer::try_setup()
        {
            SDL_Window* win = SDL_CreateWindow(
                GAME_NAME,
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                WINDOW_SIZE.x, WINDOW_SIZE.y,
                SDL_WINDOW_RESIZABLE);

            if (!win)
            {
                Log::error(SE_FUNCTION, ": Error creating SDL window: ", SDL_GetError());
                return nullptr;
            }

            SDL_Renderer* render = SDL_CreateRenderer(win, -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            // SDL2 failed to setup a hardware renderer, setting up a software renderer instead
            if (!render)
            {
                Log::warn(SE_FUNCTION, ": Error creating SDL hardware renderer: ", SDL_GetError(),
                    "; trying one more time");
                render = SDL_CreateRenderer(win, -1, 0);
            }

            // SDL2 failed to provide a renderer at all
            if (!render)
            {
                SDL_DestroyWindow(win);
                Log::error(SE_FUNCTION, ": Error creating SDL renderer: ", SDL_GetError());
                return nullptr;
            }

            int flags = IMG_INIT_PNG;

            if (IMG_Init(flags) != flags)
            {
                IMG_Quit();
                SDL_DestroyRenderer(render);
                SDL_DestroyWindow(win);
                Log::error(SE_FUNCTION, ": Error initializing SDL Image: ", IMG_GetError());
                return nullptr;
            }

            return Utils::make_unique<SDL2_Renderer>(win, render);
        }

        SDL2_Renderer::SDL2_Renderer(SDL_Window* win, SDL_Renderer* render)
            : win(win), render(render), camera_enabled(true), screen_surface(false)
        {
            SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);

            SDL_RendererInfo info;
            SDL_GetRendererInfo(render, &info);
            use_software_vsync = !strcmp(info.name, "software");

            if (SDL_RenderTargetSupported(render))
            {
                surface = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET,
                    SURFACE_SIZE.x, SURFACE_SIZE.y);
                screen_surface = false;
            }
            else
                screen_surface = true;

            surface_size = SURFACE_SIZE;

            subtract_blend_mode = SDL_ComposeCustomBlendMode(
                SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE,
                SDL_BLENDOPERATION_REV_SUBTRACT, SDL_BLENDFACTOR_SRC_ALPHA,
                SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD);

            handle_resolution_update(WINDOW_SIZE.x, WINDOW_SIZE.y);
        }

        SDL2_Renderer::~SDL2_Renderer()
        {
            IMG_Quit();
            SDL_DestroyTexture(surface);
            SDL_DestroyRenderer(render);
            SDL_DestroyWindow(win);
        }

        bool SDL2_Renderer::is_feature_supported(Feature feature)
        {
            switch (feature)
            {
                case Feature::RENDER_TARGET:
                    return SDL_RenderTargetSupported(render);

                case Feature::CAMERA_ZOOM:
                case Feature::CAMERA_ROTATION:
                case Feature::SCREENSHOT:
                    return true;

                // I want to clearly show that, unfortunately,
                // SDL2 does not support custom shaders.
                case Feature::SHADERS:
                default:
                    return false;
            }
        }

        bool SDL2_Renderer::is_blend_mode_supported(BlendMode mode)
        {
            SDL_BlendMode default_mode;
            SDL_GetRenderDrawBlendMode(render, &default_mode);

            if (!SDL_SetRenderDrawBlendMode(render, engine_to_sdl_blend(mode)))
            {
                SDL_SetRenderDrawBlendMode(render, default_mode);
                return true;
            }

            return false;
        }

        std::string SDL2_Renderer::get_name()
        {
            SDL_RendererInfo info;
            SDL_GetRendererInfo(render, &info);
            return Utils::format("SDL2 ({})", info.name);
        }

        void SDL2_Renderer::enable_camera(bool flag)
        {
            camera_enabled = flag;
            camera_zoom_or_angle = flag && ((camera_zoom != 1) || (camera_angle != 0));
        }

        void SDL2_Renderer::apply_camera(const Camera& camera)
        {
            float w = get_surface_size().x / 2.0f;
            float h = get_surface_size().y / 2.0f;

            camera_pos.x = camera.get_left() - w + w / camera.zoom;
            camera_pos.y = camera.get_up() - h + h / camera.zoom;
            camera_angle = camera.angle;
            camera_zoom = camera.zoom;

            camera_zoom_or_angle = camera_enabled && ((camera_zoom != 1) || (camera_angle != 0));
        }

        void SDL2_Renderer::set_default_scale_mode(ScaleMode mode)
        {
            default_scale_mode = mode;
        }

        IWindow& SDL2_Renderer::get_window()
        {
            return *this;
        }

        ScaleMode SDL2_Renderer::get_default_scale_mode()
        {
            return default_scale_mode;
        }

        Color SDL2_Renderer::get_bg_color()
        {
            return bg_color;
        }

        void SDL2_Renderer::set_bg_color(Color color)
        {
            bg_color = color;
        }

        bool SDL2_Renderer::set_render_target(Texture& texture)
        {
            return !SDL_SetRenderTarget(render, get_texture(texture));
        }

        void SDL2_Renderer::reset_render_target()
        {
            SDL_SetRenderTarget(render, screen_surface ? nullptr : surface);
        }

        Vector2Int SDL2_Renderer::get_surface_size()
        {
            return surface_size;
        }

        void SDL2_Renderer::set_surface_size(int w, int h)
        {
            SDL_SetRenderTarget(render, nullptr);
            SDL_RenderSetLogicalSize(render, w, h);

            if (surface)
            {
                SDL_DestroyTexture(surface);
                surface = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_TARGET, w, h);
                surface_size = { w, h };
                if (!screen_surface)
                    SDL_SetRenderTarget(render, surface);
            }
        }

        bool SDL2_Renderer::use_screen_surface(bool flag)
        {
            if (is_feature_supported(Feature::SURFACE))
            {
                screen_surface = flag;
                return true;
            }
            else
                return flag; // successful if asked for screen surface
        }

        bool SDL2_Renderer::use_window(int id)
        {
            // Should several windows be supported? I don't think so.
            return id == 0;
        }

        void SDL2_Renderer::set_primitives_blend_mode(BlendMode mode)
        {
            SDL_SetRenderDrawBlendMode(render, primitives_blend_mode = engine_to_sdl_blend(mode));
        }

        BlendMode SDL2_Renderer::get_primitives_blend_mode()
        {
            SDL_BlendMode mode;
            SDL_GetRenderDrawBlendMode(render, &mode);
            return sdl_to_engine_blend(mode);
        }

        // TODO: only clear the surface area if using screen surface
        void SDL2_Renderer::clear(Color color)
        {
            SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
            SDL_RenderClear(render);
        }

        // TODO: make primitives affected by camera
        // for that: update SDL2 to the latest update and use SDL_RenderGeometry
        void SDL2_Renderer::pixel(Vector2 pos, Color color)
        {
            SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
            SDL_RenderDrawPointF(render, pos.x, pos.y);
        }

        void SDL2_Renderer::line(Vector2 p1, Vector2 p2, Color color)
        {
            SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
            SDL_RenderDrawLineF(render, p1.x, p1.y, p2.x, p2.y);
        }

        void SDL2_Renderer::rectangle_outline(Rect rect, Color color)
        {
            SDL_Rect rect2 = rect;
            SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
            SDL_RenderDrawRect(render, &rect2);
        }

        void SDL2_Renderer::rectangle_filled(Rect rect, Color color)
        {
            SDL_Rect rect2 = rect;
            SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(render, &rect2);
        }

        void SDL2_Renderer::circle_outline(Vector2 centre, float radius, Color color)
        {
            SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
            // TODO
        }

        void SDL2_Renderer::circle_filled(Vector2 centre, float radius, Color color)
        {
            SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
            // TODO
        }

        void SDL2_Renderer::draw_texture(const Texture& texture, Vector2 pos)
        {
            Vector2 pos_int = pos,
                size = get_texture_size(texture).convert_to<float>();
            float degrees = 0;

            camera_vector(pos_int, size, degrees);

            SDL_FRect rect = { pos_int.x, pos_int.y, size.x, size.y };
            if (!camera_zoom_or_angle)
                SDL_RenderCopyF(render, get_texture(texture), nullptr, &rect);
            else
                SDL_RenderCopyExF(render, get_texture(texture), nullptr, &rect, degrees, nullptr, SDL_FLIP_NONE);
        }

        SDL_RendererFlip get_flip(Vector2& scale)
        {
            int flip = SDL_FLIP_NONE;

            if (scale.x < 0)
            {
                scale.x = -scale.x;
                flip |= SDL_FLIP_HORIZONTAL;
            }
            if (scale.y < 0)
            {
                scale.y = -scale.y;
                flip |= SDL_FLIP_VERTICAL;
            }

            return (SDL_RendererFlip)flip;
        }

        void SDL2_Renderer::draw_texture_scale(const Texture& texture, Vector2 pos, Vector2 scale)
        {
            SDL_RendererFlip flip = get_flip(scale);
            Vector2 pos_int = pos,
                size = get_texture_size(texture).convert_to<float>() * scale;
            float degrees = 0;

            camera_vector(pos_int, size, degrees);

            SDL_FRect rect = {pos_int.x, pos_int.y, size.x, size.y};
            SDL_RenderCopyExF(render, get_texture(texture), nullptr, &rect, degrees, nullptr, flip);
        }

        void SDL2_Renderer::draw_texture_rotate(const Texture& texture, Vector2 pos, float degrees)
        {
            Vector2 pos_int = pos,
                size = get_texture_size(texture).convert_to<float>();

            camera_vector(pos_int, size, degrees);

            SDL_FRect rect = { pos_int.x, pos_int.y, size.x, size.y };
            SDL_RenderCopyExF(render, get_texture(texture), nullptr, &rect, degrees, nullptr, SDL_FLIP_NONE);
        }

        void SDL2_Renderer::draw_texture_part(const Texture& texture, Vector2 pos, Rect texture_rect)
        {
            SDL_Rect tex_rect = texture_rect;
            Vector2 pos_int = pos,
                size = { (float)tex_rect.w, (float)tex_rect.h };
            float degrees = 0;

            camera_vector(pos_int, size, degrees);

            SDL_FRect rect = { pos_int.x, pos_int.y, size.x, size.y };
            if (!camera_zoom_or_angle)
                SDL_RenderCopyF(render, get_texture(texture), &tex_rect, &rect);
            else
                SDL_RenderCopyExF(render, get_texture(texture), &tex_rect, &rect,
                    degrees, nullptr, SDL_FLIP_NONE);
        }

        void SDL2_Renderer::draw_texture_transform(const Texture& texture, Vector2 pos,
                        Vector2 scale, float degrees)
        {
            SDL_RendererFlip flip = get_flip(scale);
            Vector2 pos_int = pos,
                size = get_texture_size(texture).convert_to<float>() * scale;

            camera_vector(pos_int, size, degrees);

            SDL_FRect rect = { pos_int.x, pos_int.y, size.x, size.y };
            SDL_RenderCopyExF(render, get_texture(texture),
                nullptr, &rect, degrees, nullptr, flip);
        }

        void SDL2_Renderer::draw_texture_transform(const Texture& texture, Vector2 pos,
                        Rect texture_rect, Vector2 scale, float degrees)
        {
            SDL_RendererFlip flip = get_flip(scale);
            SDL_Rect tex_rect = texture_rect;
            Vector2 pos_int = pos,
                size = Vector2{ (float)tex_rect.w, (float)tex_rect.h } * scale;

            camera_vector(pos_int, size, degrees);

            SDL_FRect rect = { pos_int.x, pos_int.y, size.x, size.y };
            SDL_RenderCopyExF(render, get_texture(texture),
                &tex_rect, &rect, degrees, nullptr, flip);
        }

        bool SDL2_Renderer::take_screenshot(const char* filename)
        {
            int width, height;
            SDL_Surface* screenshot;

            SDL_GetRendererOutputSize(render, &width, &height);
            screenshot = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
            if (!surface)
            {
                Log::error(SE_FUNCTION, ": Can't create a surface for the screenshot: ", SDL_GetError());
                return false;
            }

            if (SDL_RenderReadPixels(render, nullptr, screenshot->format->format,
                screenshot->pixels, screenshot->pitch) < 0)
            {
                Log::error(SE_FUNCTION, ": Can't read pixels for screenshot: ", SDL_GetError());
                SDL_FreeSurface(screenshot);
                return false;
            }

            if (IMG_SavePNG(screenshot, filename) < 0)
            {
                Log::error(SE_FUNCTION, ": Can't save screenshot to file '",
                    filename, "': ", IMG_GetError());
                SDL_FreeSurface(screenshot);
                return false;
            }

            SDL_FreeSurface(screenshot);
            return true;
        }

        SDL_Texture* SDL2_Renderer::get_texture(const Texture& texture)
        {
            return texture.get_const_data<SDL2_TextureData>().texture;
        }

        void SDL2_Renderer::camera_vector(Vector2& pos, Vector2& size, float& degrees)
        {
            if (camera_enabled)
            {
                pos -= camera_pos;

                if (camera_zoom_or_angle)
                {
                    Vector2 surface_centre = get_surface_size().convert_to<float>() / 2;
                    Vector2 vector = -surface_centre;
                    vector += pos;
                    vector = vector.rotate(camera_angle) * camera_zoom;
                    vector += surface_centre;

                    pos = vector;
                    size *= camera_zoom;

                    degrees += camera_angle;
                }
            }

            pos -= size / 2;
            if (!camera_zoom_or_angle)
                pos = pos.floor();
        }

        SDL_ScaleMode SDL2_Renderer::engine_to_sdl_scale(ScaleMode mode)
        {
            if (mode == ScaleMode::NEAREST)
                return SDL_ScaleModeNearest;
            else
                return SDL_ScaleModeLinear;
        }

        ScaleMode SDL2_Renderer::sdl_to_engine_scale(SDL_ScaleMode mode)
        {
            if (mode == SDL_ScaleModeNearest)
                return ScaleMode::NEAREST;
            else
                return ScaleMode::LINEAR;
        }

        SDL_BlendMode SDL2_Renderer::engine_to_sdl_blend(BlendMode mode)
        {
            switch (mode)
            {
                default:
                case BlendMode::NORMAL:
                    return SDL_BLENDMODE_BLEND;
                case BlendMode::ADD:
                    return SDL_BLENDMODE_ADD;
                case BlendMode::SUBTRACT:
                    return subtract_blend_mode;
                case BlendMode::MULTIPLY:
                    return SDL_BLENDMODE_MOD; // Intentionally not SDL_BLENDMODE_MUL
            }
        }

        BlendMode SDL2_Renderer::sdl_to_engine_blend(SDL_BlendMode mode)
        {
            if (mode == subtract_blend_mode)
                return BlendMode::SUBTRACT;

            switch (mode)
            {
                default:
                case SDL_BLENDMODE_BLEND:
                    return BlendMode::NORMAL;
                case SDL_BLENDMODE_ADD:
                    return BlendMode::ADD;
                case SDL_BLENDMODE_MOD:
                case SDL_BLENDMODE_MUL: // Just in case
                    return BlendMode::MULTIPLY;
            }
        }

        std::unique_ptr<TextureData> SDL2_Renderer::create_texture(int w, int h, bool texture_target)
        {
            return Utils::make_unique<SDL2_TextureData>(SDL_CreateTexture(
                render,
                SDL_PIXELFORMAT_RGBA32,
                texture_target ? SDL_TEXTUREACCESS_TARGET : 0,
                w, h), engine_to_sdl_scale(default_scale_mode));
        }

        std::unique_ptr<TextureData> SDL2_Renderer::create_texture(const char* filename)
        {
            return Utils::make_unique<SDL2_TextureData>(IMG_LoadTexture(render, filename),
                engine_to_sdl_scale(default_scale_mode));
        }

        std::unique_ptr<TextureData> SDL2_Renderer::create_texture(unsigned char* buffer, int size)
        {
            return Utils::make_unique<SDL2_TextureData>(IMG_LoadTexture_RW(
                render, SDL_RWFromConstMem(buffer, size), 1),
                engine_to_sdl_scale(default_scale_mode));
        }

        std::unique_ptr<TextureData> SDL2_Renderer::create_texture(SDL_Surface* surface)
        {
            return Utils::make_unique<SDL2_TextureData>(SDL_CreateTextureFromSurface(render, surface),
                engine_to_sdl_scale(default_scale_mode));
        }

        int SDL2_Renderer::get_texture_width(const Texture& texture)
        {
            return texture.get_const_data<SDL2_TextureData>().size.x;
        }

        int SDL2_Renderer::get_texture_height(const Texture& texture)
        {
            return texture.get_const_data<SDL2_TextureData>().size.y;
        }

        Vector2Int SDL2_Renderer::get_texture_size(const Texture& texture)
        {
            return texture.get_const_data<SDL2_TextureData>().size;
        }

        void SDL2_Renderer::set_texture_scale_mode(Texture& texture, ScaleMode mode)
        {
            SDL_SetTextureScaleMode(get_texture(texture), engine_to_sdl_scale(mode));
        }

        ScaleMode SDL2_Renderer::get_texture_scale_mode(const Texture& texture)
        {
            SDL_ScaleMode mode;
            SDL_GetTextureScaleMode(get_texture(texture), &mode);
            return sdl_to_engine_scale(mode);
        }

        void SDL2_Renderer::set_texture_blend_mode(Texture& texture, BlendMode mode)
        {
            SDL_SetTextureBlendMode(get_texture(texture), engine_to_sdl_blend(mode));
        }

        BlendMode SDL2_Renderer::get_texture_blend_mode(const Texture& texture)
        {
            SDL_BlendMode blend_mode;
            SDL_GetTextureBlendMode(get_texture(texture), &blend_mode);
            return sdl_to_engine_blend(blend_mode);
        }

        void SDL2_Renderer::set_texture_tint(Texture& texture, Color color)
        {
            SDL_SetTextureColorMod(get_texture(texture), color.r, color.g, color.b);
            SDL_SetTextureAlphaMod(get_texture(texture), color.a);
        }

        Color SDL2_Renderer::get_texture_tint(const Texture& texture)
        {
            Color color;
            SDL_GetTextureColorMod(get_texture(texture), &color.r, &color.g, &color.b);
            SDL_GetTextureAlphaMod(get_texture(texture), &color.a);
            return color;
        }

        void SDL2_Renderer::start_drawing()
        {
            SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
            // Black bars around the surface in case of window resizing
            SDL_Rect surface_rect = { 0, 0, get_surface_size().x, get_surface_size().y };
            SDL_SetRenderTarget(render, nullptr);
            SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
            SDL_RenderClear(render);

            // Actual surface clear code
            reset_render_target();
            SDL_SetRenderDrawColor(render, bg_color.r, bg_color.g,
                bg_color.b, bg_color.a);
            SDL_RenderFillRect(render, &surface_rect);

            SDL_SetRenderDrawBlendMode(render, primitives_blend_mode);

            if (use_software_vsync)
                frame_start = SDL_GetPerformanceCounter();
        }

        void SDL2_Renderer::end_drawing()
        {
            if (!screen_surface)
            {
                SDL_SetRenderTarget(render, nullptr);
                SDL_RenderCopy(render, surface, nullptr, nullptr);
            }

            SDL_RenderPresent(render);

            if (use_software_vsync)
            {
                Uint64 current_frame_time = SDL_GetPerformanceCounter() - frame_start;
                Uint32 ms = 1000.f * current_frame_time / SDL_GetPerformanceFrequency();
                if (ms < 16)
                    SDL_Delay(16 - ms);
            }
        }

        std::string SDL2_Renderer::get_window_name()
        {
            return SDL_GetWindowTitle(win);
        }

        void SDL2_Renderer::set_window_name(const char* name)
        {
            SDL_SetWindowTitle(win, name);
        }

        Vector2Int SDL2_Renderer::get_window_resolution()
        {
            Vector2Int size;
            SDL_GetWindowSize(win, &size.x, &size.y);
            return size;
        }

        void SDL2_Renderer::set_window_resolution(int w, int h)
        {
            SDL_SetWindowSize(win, w, h);
            handle_resolution_update(w, h);
        }

        void SDL2_Renderer::handle_resolution_update(int w, int h)
        {
            SDL_RenderSetLogicalSize(render, get_surface_size().x, get_surface_size().y);
        }

        Vector2Int SDL2_Renderer::get_window_position()
        {
            Vector2Int pos;
            SDL_GetWindowPosition(win, &pos.x, &pos.y);
            return pos;
        }

        void SDL2_Renderer::set_window_position(int x, int y)
        {
            SDL_SetWindowPosition(win, x, y);
        }

        void SDL2_Renderer::center_window()
        {
            SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }

        void SDL2_Renderer::set_resizable(bool enable)
        {
            SDL_SetWindowResizable(win, (SDL_bool)enable);
        }

        void SDL2_Renderer::set_fullscreen(bool flag)
        {
            SDL_SetWindowFullscreen(win,
                flag ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        }

        bool SDL2_Renderer::get_fullscreen()
        {
            return SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
    }
}

#endif