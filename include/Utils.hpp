#ifndef __UTILS_H__
#define __UTILS_H__

#include "Platform.hpp"
#include "Exceptions.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>

#if SE_SDL2
#include "SDL.h"
#endif

namespace Sputnik
{
    // Logging functionality
    namespace Log
	{
#if _DEBUG		
		template<typename... Args>
		void info(Args... args)
		{
			std::cout << "INFO: ";
			auto dummy = { (std::cout << args, 0)... };
			std::cout << '\n';
		}

		template<typename... Args>
		void warn(Args... args)
		{
			std::cout << "WARN: ";
			auto dummy = { (std::cout << args, 0)... };
			std::cout << '\n';
		}

		template<typename... Args>
		void error(Args... args)
		{
			std::cout << "ERROR: ";
			auto dummy = { (std::cout << args, 0)... };
			std::cout << '\n';
		}
#else
		template<typename... Args>
		void info(Args... args) {}

		template<typename... Args>
		void warn(Args... args) {}

		template<typename... Args>
		void error(Args... args)
		{
			std::stringstream ss;
			((ss << args, 0), ...);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Sputnik Error", ss.str().c_str(), nullptr);
		}
#endif
	}

    // Various utilities
    namespace Utils
    {
        /*
            Converting values to strings
        */

        template<typename T>
        std::string to_string(const T& value, char = 0)
        {
            std::ostringstream ss;
            ss << value;
            return ss.str();
        }

        inline std::string to_string(const std::string& value)
        {
            return value;
        }

        template<typename T>
        std::wstring to_string(const T& value, wchar_t)
        {
            std::wostringstream ss;
            ss << value;
            return ss.str();
        }

        template<typename T>
        std::wstring to_wstring(const T& value)
        {
            return to_string(value, (wchar_t)0);
        }

        inline std::wstring to_wstring(const std::wstring& value)
        {
            return value;
        }

        /*
            String formatting
        */

        template<typename S, typename SS, typename CHAR = typename S::value_type, typename... Args>
        S base_format(const CHAR* format_string, const Args&... args)
        {
            static_assert(sizeof(CHAR) == sizeof(char) || sizeof(CHAR) == sizeof(wchar_t),
                "Unsupported character type");

            std::vector<S> strings;
            SS stream;
            int value_id = 0;
            int length = strlen(format_string);

            strings = { to_string(args, (CHAR)0)... };

            for (int i = 0; i < length; i++)
            {
                CHAR c = format_string[i];
                if (c == (CHAR)'{' &&
                    i != (length - 1) // we don't want to read past the provided string
                    )
                {
                    i++;
                    if (format_string[i] == (CHAR)'{')
                    {
                        stream << '{';
                    }
                    else
                    {
                        stream << strings.at(value_id++);
                        if (value_id >= strings.size() - 1)
                            value_id = 0;
                    }
                }
                else
                {
                    stream << c;
                }
            }

            return stream.str();
        }

        template<typename... Args>
        std::string format(const char* format_string, const Args&... args)
        {
            return base_format<std::string, std::ostringstream>(format_string, args...);
        }

        template<typename... Args>
        std::wstring format(const wchar_t* format_string, const Args&... args)
        {
            return base_format<std::wstring, std::wostringstream>(format_string, args...);
        }

        /*
            std::make_unique and std::make_shared for objects from C++14
            and for arrays from C++20 for C++11, but still use the ones
            from standard library if available.
        */

#if __cplusplus == 201103L
        template<typename T, typename std::enable_if<!std::is_array<T>::value, int>::type = 0, typename... Args>
        std::unique_ptr<T> make_unique(const Args&... args)
        {
            std::unique_ptr<T> ptr{ new T(args...) };
            return ptr;
        }
#else
        // TODO: use C++20 concepts instead of std::enable_if
        template<typename T, typename std::enable_if<!std::is_array<T>::value, int>::type = 0, typename... Args>
        std::unique_ptr<T> make_unique(const Args&... args)
        {
            return std::make_unique<T>(args...);
        }
#endif
        template<typename T, typename std::enable_if<!std::is_array<T>::value, int>::type = 0, typename... Args>
        std::shared_ptr<T> make_shared(const Args&... args)
        {
            return std::make_shared<T>(args...);
        }

#if __cplusplus <= 201703L
        template<typename T, typename std::enable_if<std::is_array<T>::value, int>::type = 0>
        std::unique_ptr<T> make_unique(size_t size)
        {
            using T2 = typename std::remove_all_extents<T>::type;
            std::unique_ptr<T> ptr{ new T2[size] };
            return ptr;
        }

        template<typename T, typename std::enable_if<std::is_array<T>::value, int>::type = 0>
        std::shared_ptr<T> make_shared(size_t size)
        {
            using T2 = typename std::remove_all_extents<T>::type;
            std::shared_ptr<T> ptr{ new T2[size] };
            return ptr;
        }
#else
        // TODO: use C++20 concepts instead of std::enable_if
        template<typename T, typename std::enable_if<std::is_array<T>::value, int>::type = 0>
        std::unique_ptr<T> make_unique(size_t size)
        {
            return std::make_unique<T>(size);
        }

        template<typename T, typename std::enable_if<std::is_array<T>::value, int>::type = 0>
        std::shared_ptr<T> make_shared(size_t size)
        {
            return std::make_shared<T>(size);
        }
#endif

        /*
            Event handler class similar to C# event handlers
        */
        template<typename... Args>
        class EventHandler
        {
        public:
            void operator()(Args... args)
            {
                for (FunctionData& fd : data)
                    fd.f(args...);
            }

            void clear()
            {
                data.clear();
            }

            // Add a new handler, with an option to remove it later if you
            // keep this reference later
            template<typename T>
            EventHandler& operator += (T& f)
            {
                data.push_back(FunctionData{ f, (void*)&f });
                return *this;
            }

            // Add a new handler without an option to remove it later
            // (unless clear() method is used)
            template<typename T>
            EventHandler& operator += (T&& f)
            {
                data.push_back(FunctionData{ f, nullptr });
                return *this;
            }

            // Remove a handler by reference
            template<typename T>
            EventHandler& operator -= (T& f)
            {
                auto it = std::find_if(data.begin(), data.end(), [&](FunctionData& d) {
                    return (void*)&f == d.og_ptr;
                    });
                if (it != data.end())
                    data.erase(it);
                else
                    // TODO: new exception type
                    throw SputnikException("Cannot remove this function, it wasn't found");
                return *this;
            }

            // f in this case is a new function.
            // You can't remove an existing handler by referencing a newly made function
            // (code pointers may vary or captured variables can be different)
            template<typename T>
            EventHandler& operator -= (T&& f) = delete;

        private:
            struct FunctionData
            {
                std::function<void(Args...)> f;
                void* og_ptr;
            };

            std::vector<FunctionData> data;
        };
    }
}

#endif // __UTILS_H__