#include "shared/env_utils.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace DocuTrace::Shared
{
    void EnvUtils::LoadEnvFile()
    {
        const char* env_path = std::getenv("ENV_FILE");
        std::string filepath = env_path ? env_path : ".env";

        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cout << "[-] No se encontró archivo .env en " << filepath << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            // Ignorar líneas vacías o comentarios
            if (line.empty() || line[0] == '#')
                continue;

            // Buscar el signo =
            size_t pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            // Extraer key y value
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remover espacios al inicio y final
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // Establecer variable de entorno
            setenv(key.c_str(), value.c_str(), 1);
        }
    }

    std::string EnvUtils::GetEnv(const std::string& key, const std::string& default_value)
    {
        const char* value = std::getenv(key.c_str());
        return value ? value : default_value;
    }

} // namespace DocuTrace::Shared
