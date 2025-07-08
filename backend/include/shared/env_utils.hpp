#pragma once

#include <string>

namespace DocuTrace::Shared
{
    class EnvUtils
    {
      public:
        /**
         * @brief Carga archivo .env en el entorno del sistema
         */
        static void LoadEnvFile();

        /**
         * @brief Obtiene una variable de entorno con valor por defecto
         * @param key Nombre de la variable de entorno
         * @param default_value Valor por defecto si la variable no existe
         * @return Valor de la variable o valor por defecto
         */
        static std::string GetEnv(const std::string& key, const std::string& default_value = "");
    };

} // namespace DocuTrace::Shared