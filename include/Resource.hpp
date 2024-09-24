#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <string>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <memory>

namespace Sputnik
{
	class Resource
	{
	public:
		using Handle = const Resource*;

		enum class Type : int8_t
		{
			UNKNOWN = 0,
			GRAPHICS,
			SOUND,
			TEXT,
			BINARY,
		};

		static Handle get(const char* name);
		static bool add(const char* name, Resource resource);
		static bool add(const char* name, Type type, unsigned char* data, int size, bool take_ownership);
		static bool load_from_file(const char* name, Type type, const char* filename);
		static void remove(const char* name);
		static void remove_all();

		// TODO: load_resource_pack for buffers
		// TODO: encryption, set_encryption_key(const char*)
		static bool load_resource_pack(const char* filename, std::function<void(Handle)> action = nullptr);
		static void allow_overwriting(bool flag);

		// T accepts Resource::Handle as the single parameter
		template<typename T>
		static void for_each(T action)
		{
			extern std::unordered_map<std::string, Resource> resource_map;
			std::for_each(resource_map.begin(), resource_map.end(),
				[=](std::pair<const std::string, Resource>& a) {
					action(&a.second);
				});
		}

		~Resource();

		bool check_type(Type type) const;
		Type get_type() const;
		unsigned char* get_buffer() const;
		int get_size() const;
		const char* get_name() const;
		
	private:		
		Type type;
		std::shared_ptr<unsigned char[]> data;
		int size;
		std::string name;
		
		static bool add(std::string name, Type type, std::shared_ptr<unsigned char[]> data, int size);
	};
}
#endif // __RESOURCE_H__