#include "utils/env.utils.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace Utils
{
    std::string GetEnv(std::string_view varName, std::string_view defaultValue)
    {
        const char* val = std::getenv(varName.data());
        return (val && val[0] != '\0') ? std::string(val) : std::string(defaultValue);
    }

    void LoadEnvFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cout << "Warning: Could not open .env file: " << filename << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            // Saltar líneas vacías y comentarios
            if (line.empty() || line[0] == '#')
                continue;

            // Buscar el signo igual
            auto pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            // Extraer nombre y valor
            std::string name = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remover espacios en blanco
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // Remover comillas si las hay
            if (value.length() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                                        (value.front() == '\'' && value.back() == '\'')))
            {
                value = value.substr(1, value.length() - 2);
            }

            // Configurar la variable de entorno
            setenv(name.c_str(), value.c_str(), 1);
        }
    }
} // namespace Utils
