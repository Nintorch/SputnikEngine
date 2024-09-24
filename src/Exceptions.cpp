#include "Exceptions.hpp"
#include "App.hpp"
#include "Utils.hpp"
#include "Platform.hpp"

namespace Sputnik
{
	SputnikException::SputnikException(std::string message)
		: std::runtime_error(message),
		  message(message)
	{
		Log::error("Sputnik Engine error: ", message);
	}

	const char* SputnikException::what() const throw()
	{
		return message.c_str();
	}

	FileNotFoundException::FileNotFoundException(const char* filename)
		: SputnikException((std::string)"File '" + filename + "' not found")
	{
		this->filename = filename;
	}

#if SE_SDL_GPU
	SDLGPUException::SDLGPUException(GPU_ErrorObject error)
		: SputnikException((std::string)"SDL GPU error: " + error.details +
							" (from function " + error.function + ")")
	{
		error_object = error;
	}

	const char* SDLGPUException::what() const throw()
	{
		return error_object.details;
	}
#endif

	ResourceException::ResourceException(std::string name)
		: SputnikException(name) { }
}