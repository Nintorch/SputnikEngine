#include "Language.hpp"
#include "Utils.hpp"

namespace Sputnik
{
    namespace Language
    {
        struct LanguageStruct
        {
            std::wstring name;
            LanguageMap map;
        };
        
        static std::unordered_map<std::string, LanguageStruct> languages;
        static std::string current;

        void new_language(const char* id, const wchar_t* name, LanguageMap& lang_map)
        {
            auto it = languages.find(id);
            if (it != languages.end())
            {
                Log::error("Language with id '", id, "' already exists");
                return;
            }

            languages[id] = LanguageStruct{ name, lang_map };
        }
        
        const wchar_t* get_text(const char* lang_id, const char* text_id)
        {
            auto it1 = languages.find(lang_id);
            if (it1 == languages.end())
            {
                Log::error("Language with id '", lang_id, "' not found");
                return L"";
            }
            
            LanguageStruct& lang = it1->second;
            auto it2 = lang.map.find(text_id);
            if (it2 == lang.map.end())
            {
                Log::error("Language with id '", lang_id,
                    "' doesn't have a text with id '", text_id, '\'');
                return L"";
            }

            return it2->second.c_str();
        }
        
        const wchar_t* get_text(const char* text_id)
        {
            return get_text(current.c_str(), text_id);
        }
        
        void set_current(const char* lang_id)
        {
            current = lang_id;
        }
        
        const char* get_current()
        {
            return current.c_str();
        }
        
        const wchar_t* get_name(const char* lang_id)
        {
            auto it = languages.find(lang_id);
            if (it == languages.end())
            {
                Log::error("Language with id '", lang_id, "' not found");
                return L"";
            }

            return it->second.name.c_str();
        }
        
        const wchar_t* get_name()
        {
            return get_name(current.c_str());
        }
        
        std::vector<const char*> get_language_ids()
        {
            std::vector<const char*> result;
            for (auto& p : languages)
                result.push_back(p.first.c_str());
            return result;
        }
        
        std::vector<const wchar_t*> get_language_names()
        {
            std::vector<const wchar_t*> result;
            for (auto& p : languages)
                result.push_back(p.second.name.c_str());
            return result;
        }
    }
}