# DocuTrace

**DocuTrace** es un buscador de documentos locales (PDF, DOCX, TXT, etc.) con índice invertido y ranking BM25, desarrollado en C++ (backend), Tauri + React (desktop) y preparado para despliegue en Docker.

---

## 📦 Estructura del Proyecto

```
DocuTrace/
  backend/         # API REST en C++ (Crow)
  apps/
    tauri/         # Cliente de escritorio (Tauri + React)
    web/           # (Futuro) Cliente web
  docs/            # Documentación
  README.md        # Este archivo
```

---

## 🚀 Instalación Rápida

### 1. Clona el repositorio

```bash
git clone https://github.com/ReyserLyn/DocuTrace.git
cd DocuTrace
```

### 2. Compila el backend

Ver instrucciones detalladas en `backend/README.md`.

### 3. Instala y ejecuta el cliente de escritorio

Ver instrucciones detalladas en `apps/tauri/README.md`.

---

## 🐳 Despliegue con Docker

```bash
cd backend
docker build -t docutrace-backend:latest .
docker run -d -p 8000:8000 --name docutrace-backend -v $(pwd)/data:/app/data docutrace-backend:latest
```

---

## 📚 Documentación

- [Manual técnico backend](./backend/README.md)
- [Manual técnico cliente Tauri](./apps/tauri/README.md)

---

## 🛠️ Soporte

- Fedora, Ubuntu, Debian (y derivados)
- Docker para despliegue universal


