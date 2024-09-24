#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #if 0 __GNUC__
        void swap(int32_t& val)
        {
            uint32_t uint_val = __builtin_bswap32(*(uint32_t*)(&val));
            val = *(int32_t*)(&uint_val);
        }
    #elif _MSC_VER
        void swap(int32_t& val)
        {
            uint32_t uint_val = _byteswap_ulong(*(uint32_t*)(&val));
            val = *(int32_t*)(&uint_val);
        }
    #else
        #warning "Order swap is not available for this platform"
    #endif
#else
    void swap(int32_t& val) {}
#endif

class Resource
{
public:
    enum class Type : int8_t
    {
        UNKNOWN = 0,
        GRAPHICS,
        SOUND,
        TEXT,
        BINARY,
    };
    
    static inline std::vector<Resource> resources;
    
    std::string name;
    Type type;
    int32_t size;
    std::unique_ptr<unsigned char[]> data;

    static bool read(std::ifstream& file)
    {
        std::string name;
        std::getline(file, name, '\0');
        
        Type type;
        file.read((char*)&type, sizeof(type));

        int32_t size;
        file.read((char*)&size, sizeof(size));
        swap(size);

        std::unique_ptr<unsigned char[]> data =
            std::make_unique<unsigned char[]>(size);

        file.read((char*)data.get(), size);

        if (!file.good())
            return false;

        auto it = std::find_if(resources.begin(), resources.end(),
            [&](Resource& r) { return r.name == name; });

        if (it == resources.end())
            resources.emplace_back(Resource{ name, type, size, std::move(data) });
        else
            std::cout << "Warning: found a duplicate resource: '", name, "'!\n";
        
        return true;
    }

    static void read_all(std::ifstream& file)
    {
        while (read(file));
    }

    static void save_all(std::ofstream& file)
    {
        for (Resource& r : resources)
        {
            file << r.name << '\0';
            file.write((char*)&r.type, 1);
            int32_t size = r.size;
            swap(size);
            file.write((char*)&size, 4);
            file.write((char*)r.data.get(), r.size);
        }
    }
};

int create_file(const char* filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to create file '" << filename << "'.\n";
        return 1;
    }

    // 3 characters + null delimiter
    file.write("SRP", 4);

    if (file.fail())
    {
        std::cerr << "Failed to write to file '" << filename << "'.\n";
        return 1;
    }
    
    std::cout << "Successfully created an empty resource pack in '" << filename << "'!\n";
    return 0;
}

int open_file(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file '" << filename << "'.\n";
        return 1;
    }

    std::string srp_mark;
    std::string aa;
    std::getline(file, srp_mark, '\0');
    
    if (strcmp(srp_mark.c_str(), "SRP"))
    {
        std::cerr << '\'' << filename << "' is not a resource pack file.";
        return 1;
    }

    Resource::read_all(file);
    file.close();
    std::cout << "Successfully opened resource pack '" << filename << "'!\n"
        << "Resource pack contains " << Resource::resources.size() << " resources.\n";

    while (true)
    {
        int action;
        std::cout << '\n';
        std::cout <<
            R"(Available actions:
1 - List all resources
2 - Add a new resource or replace an existing one
3 - Remove a resource
4 - Exit and save
5 - Exit without saving

Choose action:)";
        std::cin >> action;
        if (std::cin.fail())
        {
            std::cerr << "Error reading from input.";
            return 1;
        }
        std::cout << '\n';
        std::cin.get();

        switch (action)
        {
            case 1:
                std::cout << "Resources: \n";
                for (int i = 0; i < Resource::resources.size(); i++)
                {
                    std::cout << i << " - " << Resource::resources.at(i).name << '\n';
                }
                break;
            case 2:
            {
                auto& resources = Resource::resources;
                std::string name;
                std::cout << "Enter a filename to create a resource from: ";
                std::getline(std::cin, name);
                
                std::ifstream resource(name, std::ios::binary);
                if (!resource.is_open())
                {
                    std::cerr << "Error opening file '" << name << "'.\n";
                    break;
                }

                std::cout << "Enter the resource name: ";
                std::getline(std::cin, name);

                auto it = std::find_if(resources.begin(), resources.end(),
                    [&](Resource& r) { return r.name == name; });
                if (it != resources.end())
                {
                    resources.erase(it);
                    std::cout << "Found a resource with the same name, replacing it.\n";
                }

                int rtype;
                std::cout << R"(Available resource types:
0 - UNKNOWN
1 - GRAPHICS
2 - SOUND
3 - TEXT
4 - BINARY
Enter the resource type:)";
                std::cin >> rtype;

                if (rtype < 0 || rtype > 4)
                {
                    std::cerr << "Invalid resource type.\n";
                    break;
                }

                Resource::Type type{ (int8_t)rtype };

                resource.seekg(0, resource.end);
                auto size = resource.tellg();
                resource.seekg(0, resource.beg);

                if (size > INT32_MAX)
                {
                    std::cerr << "Size of the file is too big to hold (" <<
                        size << ", maximum is " << INT32_MAX << ").\n";
                    break;
                }
                else if (size < 0) // Just in case
                {
                    std::cerr << "Size of the file is negative?...\n";
                    break;
                }

                std::unique_ptr<unsigned char[]> data =
                    std::make_unique<unsigned char[]>(size);

                resource.read((char*)data.get(), size);

                resources.emplace_back(Resource{ name, type, (int32_t)size, std::move(data) });
                std::cout << "Successfully added resource '" << name << "'!\n";
            }
                break;
            case 3:
            {
                auto& resources = Resource::resources;
                int id;
                std::cout << "Enter the resource id: ";
                std::cin >> id;

                if (id < 0 || id >= resources.size())
                {
                    std::cout << "Invalid ID.\n";
                    break;
                }

                std::string name = resources.at(id).name;
                resources.erase(resources.begin() + id);
                std::cout << "Successfully removed resource '" << name << "'.\n";
            }
                break;
            case 4:
            {
                std::ofstream file(filename, std::ios::binary);
                if (!file.is_open())
                {
                    std::cerr << "Error opening file '" << filename << "'.\n";
                    break;
                }
                file.write("SRP", 4);
                Resource::save_all(file);
            }
                return 0;
            case 5:
                return 0;
            default:
                std::cout << "Invalid action\n";
                break;
        }
    }
    

    return 0;
}

int show_usage(const char* filename)
{
    std::cout << "Sputnik Engine Resource Packer\n"
        << "Usage: " << filename << " [-c] {resource pack file}\n"
        << "Use -c to create a resource pack file\n";
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return show_usage(argv[0]);

    // Asked to create a resource pack
    if (argc == 3 && !strcmp(argv[1], "-c"))
        return create_file(argv[2]);

    if (argc == 2)
    {
        return open_file(argv[1]);
    }
    
    return show_usage(argv[0]);
}