#pragma once

#include <string>
#include <vector>

namespace DocuTrace::Shared
{
    class TextUtils
    {
      public:
        /**
         * @brief Divide una cadena en un vector usando un delimitador
         * @param string Cadena a dividir
         * @param delimiter Carácter delimitador
         * @return Vector con las subcadenas resultantes
         * @example splitString("hola,mundo,test", ',') → {"hola", "mundo", "test"}
         */
        static std::vector<std::string> splitString(const std::string& string, char delimiter);

        /**
         * @brief Une un vector de strings con un delimitador
         * @param vector Vector de cadenas a unir
         * @param delimiter Carácter delimitador
         * @return Cadena resultante de la unión (sin delimitador extra al final)
         * @example joinStrings({"hola", "mundo"}, ',') → "hola,mundo"
         */
        static std::string joinStrings(const std::vector<std::string>& vector, char delimiter);

        /**
         * @brief Limpia una cadena removiendo caracteres no alfanuméricos (excepto espacios)
         * @param str Cadena a limpiar
         * @return Cadena limpia con solo caracteres alfanuméricos y espacios
         * @example cleanString("Hola@#mundo!") → "Hola mundo"
         */
        static std::string cleanString(std::string str);

        /**
         * @brief Convierte un vector de strings a representación textual para debug
         * @param vector Vector a convertir
         * @param inLine Si true: formato compacto, si false: formato multi-línea
         * @return Representación en string del vector (sin coma extra al final)
         */
        static std::string vectorToString(const std::vector<std::string>& vector, bool inLine);

        /**
         * @brief Verifica si una cadena contiene solo caracteres alfanuméricos y espacios
         * @param string Cadena a verificar
         * @return true si es completamente alfanumérica (con espacios permitidos)
         * @example checkStringAlNum("test 123") → true, checkStringAlNum("test@123") → false
         */
        static bool checkStringAlNum(const std::string& string);

        /**
         * @brief Verifica si una cadena representa un número entero válido
         * @param s Cadena a verificar
         * @return true si contiene solo dígitos
         * @example isNumber("123") → true, isNumber("12a") → false
         */
        static bool isNumber(const std::string& s);

        /**
         * @brief Remueve acentos comunes del español de un texto
         * @param text Texto con acentos
         * @return Texto sin acentos (á→a, é→e, í→i, ó→o, ú→u, ñ→n)
         * @example removeSpanishAccents("José María") → "Jose Maria"
         */
        static std::string removeSpanishAccents(const std::string& text);

        /**
         * @brief Normaliza texto para búsqueda (minúsculas + sin acentos + tokenizado)
         * @param text Texto a normalizar
         * @return Vector de palabras normalizadas listas para indexación
         * @example normalizeForSearch("José María! #123") → {"jose", "maria", "123"}
         */
        static std::vector<std::string> normalizeForSearch(const std::string& text);

      private:
        /**
         * @brief Función helper para verificar si un carácter NO es alfanumérico
         * @param c Carácter a verificar
         * @return true si no es alfanumérico ni espacio
         * @note Función auxiliar usada internamente por cleanString
         */
        static bool isNotAlNum(char c);
    };

} // namespace DocuTrace::Shared