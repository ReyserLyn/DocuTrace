#include "utils/str.utils.hpp"
#include <algorithm>
#include <sstream>

std::vector<std::string> splitString(const std::string& string, char delimiter)
{
    std::vector<std::string> strings;
    std::istringstream f(string);
    std::string s;

    // Usar getline con delimitador personalizado para dividir eficientemente
    while (std::getline(f, s, delimiter))
    {
        strings.push_back(s);
    }
    return strings;
}

std::string joinStrings(const std::vector<std::string>& vector, char delimiter)
{
    std::string result;

    // Evitar agregar delimitador extra al final iterando hasta size()-1
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

bool isNotAlNum(char c)
{
    // Cast a unsigned char para evitar undefined behavior con caracteres negativos
    return std::isalnum(static_cast<unsigned char>(c)) == 0 && c != ' ';
}

bool checkStringAlNum(const std::string& string)
{
    // Early exit: retorna false en el primer carácter no válido
    for (const auto& character : string)
    {
        if (isNotAlNum(character))
            return false;
    }
    return true;
}

bool isNumber(const std::string& s)
{
    // find_if retorna end() si todos los caracteres son dígitos
    return !s.empty() && std::find_if(s.begin(), s.end(),
                                      [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

std::string cleanString(std::string str)
{
    // Erase-remove idiom: remove_if marca elementos, erase los elimina
    str.erase(std::remove_if(str.begin(), str.end(), isNotAlNum), str.end());
    return str;
}

std::string vectorToString(const std::vector<std::string>& vector, bool inLine)
{
    std::string result = "[";

    if (!inLine)
        result += "\n";

    // Mismo patrón que joinStrings: evitar coma extra al final
    for (size_t i = 0; i < vector.size(); ++i)
    {
        result += vector[i];

        if (i < vector.size() - 1)
        {
            result += inLine ? ", " : ",\n";
        }
    }

    // Agregar nueva línea final solo si no está en línea y hay elementos
    if (!inLine && !vector.empty())
        result += "\n";

    result += "]";
    return result;
}
