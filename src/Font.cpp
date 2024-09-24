#include "Font.hpp"
#include "App.hpp"
#include "Utils.hpp"

namespace Sputnik
{
#if SE_SDL2
    FontTTF::FontTTF(const char* filename, int ptsize)
    {
        font = TTF_OpenFont(filename, ptsize);
        if (!font)
            Log::error("Cannot load font '", filename, "' (size=", ptsize, ')');
    }
    
    FontTTF::FontTTF(unsigned char* buffer, int size, int ptsize)
    {
        font = TTF_OpenFontRW(SDL_RWFromConstMem(buffer, size), 1, ptsize);
        if (!font)
            Log::error("Cannot load font from buffer ", (void*)buffer,
                " (buffer size=", size, ", point size=", ptsize, ')');
    }
    
    FontTTF::FontTTF(Resource::Handle resource, int ptsize)
        : FontTTF(resource->get_buffer(), resource->get_size(), ptsize) {}

    FontTTF::~FontTTF()
    {
        TTF_CloseFont(font);
    }

    Texture FontTTF::render_text(const char* text, Color color)
    {
        Texture img;
        SDL_Surface* surf = TTF_RenderText_Blended(font, text, color);
        img.load_from_SDL_surface(surf);
        SDL_FreeSurface(surf);
        return img;
    }

    Texture FontTTF::render_unicode(const wchar_t* text, Color color)
    {
        Texture img;
#if SE_WINDOWS
        // wchar_t on Windows is 16 bits
        SDL_Surface* surf = TTF_RenderUNICODE_Blended(font, (const Uint16*)text, color);
#endif // TODO: other platforms like Linux
        img.load_from_SDL_surface(surf);
        SDL_FreeSurface(surf);
        return img;
    }
#endif
}