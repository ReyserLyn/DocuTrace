#include "shared/text_utils.hpp"
#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>

namespace DocuTrace::Shared
{
    std::vector<std::string> TextUtils::splitString(const std::string& string, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(string);
        std::string token;

        while (std::getline(ss, token, delimiter))
        {
            if (!token.empty())
            {
                tokens.push_back(token);
            }
        }

        return tokens;
    }

    std::string TextUtils::joinStrings(const std::vector<std::string>& vector, char delimiter)
    {
        std::string result;
        for (size_t i = 0; i < vector.size(); ++i)
        {
            result += vector[i];
            if (i < vector.size() - 1)
            {
                result += delimiter;
            }
        }
        return result;
    }

    std::string TextUtils::cleanString(std::string str)
    {
        str.erase(std::remove_if(str.begin(), str.end(),
                                 [](char c) { return !std::isalnum(c) && !std::isspace(c); }),
                  str.end());
        return str;
    }

    std::string TextUtils::vectorToString(const std::vector<std::string>& vector, bool inLine)
    {
        std::string delimiter = inLine ? ", " : "\n";
        return joinStrings(vector, delimiter[0]);
    }

    bool TextUtils::checkStringAlNum(const std::string& string)
    {
        return std::all_of(string.begin(), string.end(),
                           [](char c) { return std::isalnum(c) || std::isspace(c); });
    }

    bool TextUtils::isNumber(const std::string& s)
    {
        return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
    }

    std::string TextUtils::removeSpanishAccents(const std::string& text)
    {
        // Mapa de caracteres UTF-8 a ASCII
        static const std::map<std::string, char> accents = {
            {"\xC3\xA1", 'a'}, // á
            {"\xC3\xA9", 'e'}, // é
            {"\xC3\xAD", 'i'}, // í
            {"\xC3\xB3", 'o'}, // ó
            {"\xC3\xBA", 'u'}, // ú
            {"\xC3\x81", 'A'}, // Á
            {"\xC3\x89", 'E'}, // É
            {"\xC3\x8D", 'I'}, // Í
            {"\xC3\x93", 'O'}, // Ó
            {"\xC3\x9A", 'U'}, // Ú
            {"\xC3\xB1", 'n'}, // ñ
            {"\xC3\x91", 'N'}  // Ñ
        };

        std::string result;
        result.reserve(text.length());

        for (size_t i = 0; i < text.length();)
        {
            bool found = false;
            // Intentar caracteres UTF-8 de 2 bytes
            if (i + 1 < text.length())
            {
                std::string two_bytes = text.substr(i, 2);
                auto it = accents.find(two_bytes);
                if (it != accents.end())
                {
                    result += it->second;
                    i += 2;
                    found = true;
                }
            }
            if (!found)
            {
                result += text[i];
                i++;
            }
        }

        return result;
    }

    std::vector<std::string> TextUtils::normalizeForSearch(const std::string& text)
    {
        // 1. Convertir a minúsculas
        std::string normalized = text;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);

        // 2. Remover acentos
        normalized = removeSpanishAccents(normalized);

        // 3. Limpiar caracteres no alfanuméricos (excepto espacios)
        normalized = cleanString(normalized);

        // 4. Tokenizar por espacios
        return splitString(normalized, ' ');
    }

} // namespace DocuTrace::Shared
