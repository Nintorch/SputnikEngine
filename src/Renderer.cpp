#include "Renderer.hpp"

#include "Platform.hpp"
#include "Exceptions.hpp"
#include "Resource.hpp"
#include "Utils.hpp"

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

	namespace Renderer
	{
		TextureData::TextureData()
		{
			Log::info("Texture ", this, " loaded");
			ALLOCATED;
		}

		TextureData::~TextureData()
		{
			Log::info("Texture ", this, " freed");
			DEALLOCATED;
		}
	}

	Texture::Texture(Texture&& img)
	{
		data = std::move(img.data);
	}

	void Texture::create(int w, int h, bool texture_target)
	{
		data = get_current_renderer().create_texture(w, h, texture_target);
	}

	void Texture::load_from_file(const char* filename)
	{
		data = get_current_renderer().create_texture(filename);
	}

	void Texture::load_from_buffer(unsigned char* buffer, int size)
	{
		data = get_current_renderer().create_texture(buffer, size);
	}

	void Texture::load_from_resource(Resource::Handle resource)
	{
		if (resource == nullptr)
		{
			Log::error(SE_FUNCTION, ": resource is null");
			return;
		}
		
		if (!resource->check_type(Resource::Type::GRAPHICS))
		{
			Log::error(SE_FUNCTION, ": Resource '", resource->get_name(),
				"' is not a graphics resource (type: ", (int)resource->get_type(), ")");
			return;
		}

		load_from_buffer(resource->get_buffer(), resource->get_size());
	}

#if SE_SDL2
	void Texture::load_from_SDL_surface(SDL_Surface* surface)
	{
		data = get_current_renderer().create_texture(surface);
	}
#endif
	
	Texture& Texture::operator = (Texture&& img)
	{
		data = std::move(img.data);

		return *this;
	}

	void Texture::free()
	{
		data.reset();
	}

	int Texture::get_width() const
	{
		return get_current_renderer().get_texture_width(*this);
	}

	int Texture::get_height() const
	{
		return get_current_renderer().get_texture_height(*this);
	}

	Vector2Int Texture::get_size() const
	{
		return get_current_renderer().get_texture_size(*this);
	}
	
	void Texture::set_scale_mode(Renderer::ScaleMode mode)
	{
		get_current_renderer().set_texture_scale_mode(*this, mode);
	}
	
	Renderer::ScaleMode Texture::get_scale_mode() const
	{
		return get_current_renderer().get_texture_scale_mode(*this);
	}
	
	void Texture::set_blend_mode(Renderer::BlendMode mode)
	{
		get_current_renderer().set_texture_blend_mode(*this, mode);
	}
	
	Color Texture::get_tint() const
	{
		return get_current_renderer().get_texture_tint(*this);
	}
	
	Renderer::BlendMode Texture::get_blend_mode() const
	{
		return get_current_renderer().get_texture_blend_mode(*this);
	}
	
	void Texture::set_tint(Color color)
	{
		get_current_renderer().set_texture_tint(*this, color);
	}

	int Texture::get_allocated_count()
	{
		return GET_ALLOCATED();
	}
}