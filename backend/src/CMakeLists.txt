# Recolecta todos los archivos fuente de forma recursiva
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Crea el ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Incluye directorios necesarios
target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

# Vincula bibliotecas externas
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  ZLIB::ZLIB
  OpenSSL::SSL
  Crow::Crow
)

# Opciones de compilador por tipo de build
target_compile_options(${PROJECT_NAME}
  PRIVATE
  -Wall
  -Wpedantic
  $<$<CONFIG:Release>:-O2>
  $<$<CONFIG:Debug>:-g>
  $<$<CONFIG:RelWithDebInfo>:-O2 -g>
  $<$<CONFIG:MinSizeRel>:-Os>
)

# Copiar archivo .env al directorio de salida del ejecutable
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_SOURCE_DIR}/.env"
  "$<TARGET_FILE_DIR:${PROJECT_NAME}>/.env"
  COMMENT "Copiando archivo .env al directorio del ejecutable"
)

# Mensaje útil al configurar
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Cabeceras precompiladas
target_precompile_headers(${PROJECT_NAME}
  PRIVATE
  <string>
  <string_view>
  <format>
  <regex>
  <optional>
  <utility>
  <functional>
  <memory>
  <mutex>
  <exception>
  <vector>
  <unordered_map>
  <algorithm>
  <chrono>
)
