#pragma once

#include <string>
#include <vector>

/**
 * @brief Divide una cadena en un vector usando un delimitador
 * @param string Cadena a dividir
 * @param delimiter Carácter delimitador
 * @return Vector con las subcadenas resultantes
 * @example splitString("hola,mundo,test", ',') → {"hola", "mundo", "test"}
 */
std::vector<std::string> splitString(const std::string& string, char delimiter);

/**
 * @brief Une un vector de strings con un delimitador
 * @param vector Vector de cadenas a unir
 * @param delimiter Carácter delimitador
 * @return Cadena resultante de la unión (sin delimitador extra al final)
 * @example joinStrings({"hola", "mundo"}, ',') → "hola,mundo"
 */
std::string joinStrings(const std::vector<std::string>& vector, char delimiter);

/**
 * @brief Limpia una cadena removiendo caracteres no alfanuméricos (excepto espacios)
 * @param str Cadena a limpiar
 * @return Cadena limpia con solo caracteres alfanuméricos y espacios
 * @example cleanString("Hola@#mundo!") → "Hola mundo"
 */
std::string cleanString(std::string str);

/**
 * @brief Convierte un vector de strings a representación textual para debug
 * @param vector Vector a convertir
 * @param inLine Si true: formato compacto, si false: formato multi-línea
 * @return Representación en string del vector (sin coma extra al final)
 * @example
 *   vectorToString({"a", "b"}, true) → "[a, b]"
 *   vectorToString({"a", "b"}, false) → "[\n  a,\n  b\n]"
 */
std::string vectorToString(const std::vector<std::string>& vector, bool inLine);

/**
 * @brief Verifica si una cadena contiene solo caracteres alfanuméricos y espacios
 * @param string Cadena a verificar
 * @return true si es completamente alfanumérica (con espacios permitidos)
 * @example checkStringAlNum("test 123") → true, checkStringAlNum("test@123") → false
 */
bool checkStringAlNum(const std::string& string);

/**
 * @brief Verifica si una cadena representa un número entero válido
 * @param s Cadena a verificar
 * @return true si contiene solo dígitos
 * @example isNumber("123") → true, isNumber("12a") → false
 */
bool isNumber(const std::string& s);

/**
 * @brief Función helper para verificar si un carácter NO es alfanumérico
 * @param c Carácter a verificar
 * @return true si no es alfanumérico ni espacio
 * @note Función auxiliar usada internamente por cleanString
 */
bool isNotAlNum(char c);