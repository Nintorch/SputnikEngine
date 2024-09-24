#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include "Platform.hpp"

#if SE_SDL_GPU
#include "SDL_gpu.h"
#endif

#include <string>
#include <stdexcept>

namespace Sputnik
{
	class SputnikException : public std::runtime_error
	{
	public:
		SputnikException(std::string message);
		virtual const char* what() const throw() override;

		std::string message;
	};

	class FileNotFoundException : public SputnikException
	{
	public:
		FileNotFoundException(const char* filename);

		std::string filename;
	};

#if SE_SDL_GPU
	class SDLGPUException : public SputnikException
	{
	public:
		SDLGPUException(GPU_ErrorObject error);
		const char* what() const throw() override;

		GPU_ErrorObject error_object;
	};
#endif

	class ResourceException : public SputnikException
	{
	public:
		ResourceException(std::string message);
	};
}
#endif // __EXCEPTIONS_H__