#ifndef __FONT_H__
#define __FONT_H__

#include "Renderer.hpp"
#include "Color.hpp"
#include "Platform.hpp"
#include "Resource.hpp"

#if SE_SDL2
#include "SDL_ttf.h"
#endif

// Check if TTF font is available
#define SE_TTF_AVAILABLE (SE_SDL2)

namespace Sputnik
{
    class IFont
    {
    public:
        virtual ~IFont() = default;
        virtual Texture render_text(const char* text, Color color) = 0;
        virtual Texture render_unicode(const wchar_t* text, Color color) = 0;
    };

#if SE_TTF_AVAILABLE
    class FontTTF : public IFont
    {
    public:
        // TODO: multiline render
        FontTTF(const char* filename, int ptsize);
        FontTTF(unsigned char* buffer, int size, int ptsize);
        FontTTF(Resource::Handle resource, int ptsize);
        ~FontTTF();
        
        Texture render_text(const char* text, Color color) override;
        Texture render_unicode(const wchar_t* text, Color color) override;

    protected:
    #if SE_SDL2
        TTF_Font* font;
    #endif // SE_SDL2
    };
#endif // SE_TTF_AVAILABLE

}

#endif // __FONT_H__