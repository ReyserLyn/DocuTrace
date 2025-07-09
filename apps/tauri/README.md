# Manual Técnico del Cliente de Escritorio - DocuTrace

## 1. Descripción General

Este documento proporciona las instrucciones técnicas para configurar, ejecutar y compilar el cliente de escritorio de DocuTrace. La aplicación está construida con **Tauri**, lo que le permite combinar un backend de Rust ligero con una interfaz de usuario moderna y reactiva desarrollada en **React**, **TypeScript** y **Tailwind CSS**.

---

## 2. Requisitos del Sistema

La instalación requiere un entorno de desarrollo para Rust y Node.js.

### 2.1. Entorno de Desarrollo

- **Node.js**: Versión 18 o superior.
- **Gestor de Paquetes**: `pnpm` (recomendado).
- **Entorno Rust**: Instalado a través de `rustup`.
- **Tauri CLI**: Interfaz de línea de comandos de Tauri.
- **Control de Versiones**: `git`.

### 2.2. Dependencias del Sistema Operativo (Crucial para Linux)

Tauri depende de `webkit` para renderizar la interfaz. Es **obligatorio** instalar las siguientes dependencias de desarrollo.

#### Para Fedora:
```bash
sudo dnf install -y webkit2gtk4.1-devel \
                      build-essential \
                      curl \
                      wget \
                      file \
                      openssl-devel \
                      libgtk-3-devel \
                      libayatana-appindicator-devel \
                      librsvg2-devel
```

#### Para Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y libwebkit2gtk-4.0-dev \
                      build-essential \
                      curl \
                      wget \
                      file \
                      libssl-dev \
                      libgtk-3-dev \
                      libayatana-appindicator3-dev \
                      librsvg2-dev
```

---

## 3. Instalación y Ejecución

Sigue estos pasos para ejecutar la aplicación en modo de desarrollo.

### Paso 3.1: Instalar Herramientas Base

Si no las tienes, instala `rustup` (que gestiona Rust), `pnpm` y `tauri-cli`.

```bash
# Instalar Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source "$HOME/.cargo/env"

# Instalar pnpm (gestor de paquetes Node)
npm install -g pnpm

# Instalar Tauri CLI
cargo install tauri-cli
```

### Paso 3.2: Instalar Dependencias del Proyecto

Navega al directorio de Tauri y usa `pnpm` para instalar las librerías de Node.js.

```bash
cd apps/tauri
pnpm install
```

### Paso 3.3: Ejecutar en Modo Desarrollo

Este comando compilará la aplicación y la abrirá en una ventana de escritorio con hot-reloading activado para el frontend.

```bash
# Asegúrate de que el backend esté corriendo en http://localhost:8000
pnpm tauri dev
```

---

## 4. Compilación para Producción

Para crear un ejecutable de producción (ej: `.deb`, `.AppImage`):

```bash
# Desde el directorio apps/tauri
pnpm tauri build
```
Los artefactos compilados se encontrarán en `apps/tauri/src-tauri/target/release/bundle/`.

---

## 5. Configuración del Entorno

La aplicación se configura mediante un fichero `.env` en el directorio `apps/tauri`.

Crea un fichero `.env` con el siguiente contenido:
```env
# URL donde se está ejecutando el backend de DocuTrace
VITE_BACKEND_URL=http://localhost:8000

# Contraseña para acceder al panel de administración (opcional)
VITE_ADMIN_PASSWORD=admin
```

---

## 6. Estructura del Proyecto

- `src/`: Código fuente del frontend en React y TypeScript.
  - `components/`: Componentes de UI reutilizables (construidos con shadcn/ui).
  - `views/`: Vistas principales de la aplicación (Home, Admin, Settings).
  - `hooks/`: Hooks de React personalizados para la lógica de la aplicación.
  - `services/`: Capa de servicio para comunicarse con la API del backend.
  - `store/`: Almacenes de estado global con Zustand.
  - `lib/`: Utilidades generales (ej: `cn` para clases, cliente TanStack).
- `src-tauri/`: Código y configuración del backend de Tauri en Rust.
  - `Cargo.toml`: Dependencias de Rust.
  - `tauri.conf.json`: Fichero de configuración principal de Tauri.
- `public/`: Recursos estáticos.
- `pnpm-lock.yaml`: Fichero de bloqueo de dependencias de pnpm.

---

## 7. Solución de Problemas

- **Error: "Backend desconectado"**:
  La causa más común es que el backend de C++ no se está ejecutando. Inícialo y reinicia la aplicación Tauri. Verifica también que la URL en tu fichero `.env` sea correcta.

- **Error al compilar (Build Error)**:
  Asegúrate de haber instalado todas las dependencias del sistema operativo listadas en la sección 2.2. Un error de `webview` o `gtk` casi siempre indica que falta una de estas librerías.

- **Comando `pnpm` no encontrado**:
  Asegúrate de haber instalado pnpm globalmente con `npm install -g pnpm`.
