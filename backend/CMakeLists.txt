cmake_minimum_required(VERSION 3.14)

project(docutrace-backend LANGUAGES CXX)

# Estándar de C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Exportar compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Tipo de build por defecto
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Rutas de salida organizadas en la carpeta de build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ========================
# Dependencias externas (por vcpkg)
# ========================
find_package(Crow CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# ========================
# Rutas de includes
# ========================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ========================
# Subdirectorios
# ========================
add_subdirectory(src)

