# Manual Técnico del Backend - DocuTrace

## 1. Descripción General

Este documento proporciona instrucciones técnicas detalladas para la configuración, compilación y ejecución del backend de DocuTrace. El backend consiste en una API REST de alto rendimiento, implementada en C++17 con el framework Crow, diseñada para la indexación y búsqueda de documentos mediante un motor BM25 propietario.

---

## 2. Requisitos del Sistema

### 2.1. Dependencias Principales

- **Compilador C++**: `g++` versión 11 o superior (con soporte para C++17).
- **Sistema de Compilación**: `cmake` versión 3.14 o superior.
- **Control de Versiones**: `git`.
- **Herramienta de Build**: `make`.

### 2.2. Gestor de Paquetes: vcpkg (Obligatorio)

El proyecto utiliza `vcpkg` para gestionar las dependencias de C++. Su instalación y configuración son **obligatorias** para una compilación exitosa.

#### Paso 2.2.1: Instalar vcpkg

Si no tienes `vcpkg`, clónalo e instálalo en una ubicación estable (ej: tu directorio home `/opt`).

```bash
# Clona el repositorio de vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Ejecuta el script de bootstrap
./bootstrap-vcpkg.sh
```

#### Paso 2.2.2: Configurar la variable de entorno VCPKG_ROOT

El sistema de compilación necesita saber dónde está instalado `vcpkg`. Define la variable de entorno `VCPKG_ROOT`.

**Para la sesión actual:**
```bash
export VCPKG_ROOT=/ruta/a/tu/vcpkg
```

**Para que sea permanente (recomendado):**

Añade la línea `export` a tu archivo de configuración de shell (`~/.bashrc`, `~/.zshrc`, etc.) y recárgalo.

```bash
# Ejemplo para .zshrc
echo 'export VCPKG_ROOT=/ruta/a/tu/vcpkg' >> ~/.zshrc
source ~/.zshrc

# Ejemplo para .bashrc
echo 'export VCPKG_ROOT=/ruta/a/tu/vcpkg' >> ~/.bashrc
source ~/.bashrc
```

---

## 3. Compilación e Instalación

Sigue estos pasos para compilar el backend desde el código fuente.

### Step 3.1: Instalar Dependencias del Sistema

#### Para Fedora:
```bash
sudo dnf install -y git cmake gcc-c++ make
```

#### Para Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y git cmake g++ make
```

### Step 3.2: Instalar Dependencias del Proyecto con vcpkg

Desde el directorio `backend`, usa `vcpkg` para instalar las librerías definidas en `vcpkg.json`.

```bash
cd backend
vcpkg install
```
Este comando descargará y compilará `crow`, `nlohmann-json`, `openssl` y `zlib`.

### Step 3.3: Configurar y Compilar con CMake

Una vez instaladas las dependencias, configura el proyecto con CMake, asegurándote de apuntar al archivo de toolchain de `vcpkg`.

```bash
# Crea un directorio de compilación
mkdir -p build
cd build

# Configura el proyecto con CMake y el toolchain de vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# Compila el proyecto usando todos los núcleos de procesador disponibles
make -j$(nproc)
```

El binario compilado se encontrará en `backend/build/bin/docutrace-backend`.

---

## 4. Ejecución

### 4.1. Ejecución Local

Tras una compilación exitosa:
```bash
# Desde el directorio backend/build/bin
./docutrace-backend
```
Por defecto, la API se iniciará en `http://localhost:8000`.

### 4.2. Ejecución con Docker (Recomendado para Despliegue)

El `Dockerfile` proporciona un entorno de producción consistente.

```bash
# Navega al directorio backend
cd backend

# Construye la imagen Docker
docker build -t docutrace-backend:latest .

# Ejecuta el contenedor
docker run -d -p 8000:8000 --name docutrace-backend -v "$(pwd)/data:/app/data" docutrace-backend:latest
```
Este comando mapea el puerto 8000 y crea un volumen persistente para los datos de los documentos.

---

## 5. Endpoints de la API

Puedes probar la API en ejecución con `curl`:

- **Ping (Verificar si está activo):**
  ```bash
  curl http://localhost:8000/ping
  ```
- **Health Check (Estado detallado):**
  ```bash
  curl http://localhost:8000/health
  ```
- **Subir Documento:**
  ```bash
  # Reemplaza con una ruta real a un archivo .txt
  curl -F 'file=@/ruta/a/tu/documento.txt' http://localhost:8000/api/upload
  ```
- **Buscar:**
  ```bash
  # Reemplaza 'palabra_clave' con tu término de búsqueda
  curl 'http://localhost:8000/api/search?query=palabra_clave'
  ```

---

## 6. Estructura del Proyecto

- `src/`: Ficheros fuente de C++ (`.cpp`).
  - `controllers/`: Manejan las peticiones HTTP.
  - `services/`: Contienen la lógica de negocio.
  - `infrastructure/`: Motor BM25, implementación del índice.
  - `shared/`: Funciones de utilidad.
- `include/`: Ficheros de cabecera de C++ (`.hpp`).
- `logs/`: Los ficheros de log en tiempo de ejecución se crearán aquí.
- `data/`: Área de almacenamiento para documentos y metadatos (creada en tiempo de ejecución).
- `Dockerfile`: Define la imagen de producción de Docker.
- `CMakeLists.txt`: Script de compilación principal de CMake.
- `vcpkg.json`: Lista las dependencias de C++ para vcpkg.

---

## 7. Solución de Problemas

- **Error: `Could not find a package configuration file...`**:
  Este error casi siempre significa que `CMAKE_TOOLCHAIN_FILE` no se pasó correctamente a CMake. Asegúrate de que estás usando el flag `-DCMAKE_TOOLCHAIN_FILE=...` y que `VCPKG_ROOT` está definido.

- **Error: `VCPKG_ROOT` no definido**:
  La variable de entorno no está definida. Ejecuta `export VCPKG_ROOT=/ruta/a/vcpkg` o añádela a tu `.zshrc`/`.bashrc`.

- **Permisos Denegados para la carpeta `data/`**:
  La aplicación necesita permisos de escritura en su directorio de trabajo para crear la carpeta `data`. Ejecuta `sudo chmod -R 777 .` en la raíz del proyecto si es necesario (solo para desarrollo local).

