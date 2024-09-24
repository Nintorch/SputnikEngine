#include "Exceptions.hpp"
#include "Resource.hpp"
#include "Utils.hpp"

#include <fstream>
#include <iostream>

#define LOG_PREFIX "RESOURCE: "

namespace Sputnik
{
	std::unordered_map<std::string, Resource> resource_map;
	static bool overwriting_allowed = false;

	Resource::Handle Resource::get(const char* name)
	{
		auto it = resource_map.find(name);
		if (it != resource_map.end())
			return &it->second;
		else
		{
			Log::error(LOG_PREFIX "Resource '", name, "' doesn't exist");
			return nullptr;
		}
	}

	bool Resource::add(const char* name, Resource resource)
	{
		auto it = resource_map.find(name);
		if (it != resource_map.end())
		{
			if (!overwriting_allowed)
			{
				Log::error(LOG_PREFIX "Tried to overwrite resource '", name,
					"' (overwriting not allowed)");
				return false;
			}

			if (it->second.type == resource.type)
			{
				Log::info(LOG_PREFIX "Resource '", name, "' was overwritten");
			}
			else
			{
				Log::error(LOG_PREFIX "Tried to overwrite resource '", name,
					"' with different type (from ", (int)it->second.type,
					" to ", (int)resource.type);
				return false;
			}
		}
		resource_map[name] = resource;

		Log::info(LOG_PREFIX "Added resource '", name, "' (buffer at ",
			(void*)resource.data.get(), ", size = ", resource.size, ')');
		return true;
	}

	bool Resource::add(const char* name, Type type, unsigned char* data, int size, bool take_ownership)
	{
		std::shared_ptr<unsigned char[]> ptr;

		if (take_ownership)
			ptr = std::shared_ptr<unsigned char[]>(data);
		else
			ptr = std::shared_ptr<unsigned char[]>(data, [](unsigned char* a) {});
		return add(std::string(name), type, ptr, size);
	}

	bool Resource::add(std::string name, Type type, std::shared_ptr<unsigned char[]> data, int size)
	{
		Resource r;
		r.type = type;
		r.data = data;
		r.size = size;
		r.name = name;
		return add(name.c_str(), std::move(r));
	}

	bool Resource::load_from_file(const char* name, Type type, const char* filename)
	{
		std::ifstream file(filename, std::ios::binary);

		if (!file.is_open())
		{
			Log::error(LOG_PREFIX "Error opening resource file '", filename, '\'');
			return false;
		}

		std::streampos start, size;
		start = file.tellg();
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);
		
		if (size < 0)
		{
			Log::error(LOG_PREFIX "Error while getting file size for resource file '",
				filename, '\'');
			return false;
		}

		std::shared_ptr<unsigned char[]> data =
			Utils::make_shared<unsigned char[]>(size);
		file.read((char*)data.get(), size);

		if (file.fail())
		{
			Log::error(LOG_PREFIX "Error reading resource file '", filename, '\'');
			return false;
		}

		file.close();

		return add(std::string(name), type, data, (int)size);
	}

	void Resource::remove(const char* name)
	{
		std::string s = std::string(name);
		auto it = resource_map.find(s);
		if (it != resource_map.end())
			resource_map.erase(s);
	}

	void Resource::remove_all()
	{
		resource_map.clear();
	}

	bool Resource::load_resource_pack(const char* filename, std::function<void(Resource::Handle)> action)
	{
		std::ifstream rp(filename, std::ios::binary);
		if (!rp.is_open())
		{
			Log::error(LOG_PREFIX "Can't open resource pack '", filename, '\'');
			return false;
		}

		char srp_mark[4];
		rp.read(srp_mark, 4);
		if (strcmp("SRP", srp_mark) != 0)
		{
			Log::error(LOG_PREFIX "Invalid resource pack '", filename,
				"' (magic key does not match)");
			return false;
		}

		Log::info(LOG_PREFIX "Loading resource pack '", filename, '\'');

		std::string resource_name;
		Resource::Type type = Resource::Type::UNKNOWN;
		Sint32 resource_size = 0;

		while (std::getline(rp, resource_name, '\0'))
		{
			rp.read((char*)&type, 1);
			rp.read((char*)&resource_size, 4);

			if (resource_size < 0)
			{
				Log::error(LOG_PREFIX "Resource '", resource_name, "' is too big");
				return false;
			}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			resource_size = SDL_Swap32(resource_size);
#endif

			std::shared_ptr<unsigned char[]> data =
				Utils::make_shared<unsigned char[]>(resource_size);
			rp.read((char*)data.get(), resource_size);

			if (!Resource::add(resource_name, type, data, resource_size))
				/* the error is already logged */
				return false;

			if (action)
				action(Resource::get(resource_name.c_str()));
		}

		rp.close();
		return true;
	}

	void Resource::allow_overwriting(bool flag)
	{
		overwriting_allowed = flag;
	}
	
	Resource::~Resource()
	{
		// data is about to be freed
		if (data && data.use_count() == 1)
			Log::info(LOG_PREFIX "Resource '", name, "' freed");
	}
	
	bool Resource::check_type(Type type) const
	{
		return this->type == type;
	}
	
	Resource::Type Resource::get_type() const
	{
		return type;
	}
	
	unsigned char* Resource::get_buffer() const
	{
		return data.get();
	}
	
	int Resource::get_size() const
	{
		return size;
	}
	
	const char* Resource::get_name() const
	{
		return name.c_str();
	}
}