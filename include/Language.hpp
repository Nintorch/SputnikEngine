#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include <unordered_map>
#include <vector>
#include <string>

namespace Sputnik
{
    namespace Language
    {
        // Map from text ID to text in the language
        // TODO: allow creating LanguageMap from a const buffer without copying
        using LanguageMap = std::unordered_map<std::string, std::wstring>;

        // Creates a new language with the specified ID, name, and map of texts
        void new_language(const char* id, const wchar_t* name, LanguageMap& lang_map);

        // Gets the text with the specified ID in the specified language
        const wchar_t* get_text(const char* lang_id, const char* text_id);
        // Gets the text with the specified ID in the current language
        const wchar_t* get_text(const char* text_id);

        // Sets the current language to the specified ID
        void set_current(const char* lang_id);
        // Gets the ID of the current language
        const char* get_current();
        // Gets the name of the language with the specified ID
        const wchar_t* get_name(const char* lang_id);
        // Gets the name of the current language
        const wchar_t* get_name();

        std::vector<const char*> get_language_ids();
        std::vector<const wchar_t*> get_language_names();
    }
}

#endif // __LANGUAGE_H__