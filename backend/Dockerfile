# Stage 1: Build
FROM csantve/alpine-vcpkg AS builder

# Argumentos de build con valores por defecto
ARG BUILD_TYPE=Release
ARG PORT=8000

WORKDIR /app

# Instalar dependencias necesarias para el build
RUN apk add --no-cache \
  linux-headers \
  perl \
  bash \
  pkgconfig \
  musl-dev \
  gcc \
  g++

# Copiar archivos de dependencias primero (para aprovechar cache de Docker)
COPY vcpkg.json .

# Instalar dependencias de vcpkg
RUN vcpkg install

# Copiar código fuente
COPY include/ ./include/
COPY src/ ./src/
COPY CMakeLists.txt .

# Configurar y compilar
RUN cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_CXX_FLAGS="-static-libgcc -static-libstdc++"

RUN cmake --build build --config ${BUILD_TYPE} --parallel $(nproc)

# Stage 2: Runtime
FROM alpine:latest

# Variables de entorno con valores por defecto
ENV PORT=8000
ENV BUILD_TYPE=Release

# Instalar solo las dependencias runtime necesarias
RUN apk add --no-cache \
  libstdc++ \
  ca-certificates \
  tzdata \
  curl

# Crear usuario no-root para seguridad
RUN addgroup -g 1001 docutrace && \
  adduser -D -s /bin/sh -u 1001 -G docutrace docutrace

WORKDIR /app

# Copiar solo el binario desde el builder
COPY --from=builder /app/build/bin/docutrace-backend ./docutrace-backend

# Crear directorio de datos y dar permisos
RUN mkdir -p /app/data && \
  chown -R docutrace:docutrace /app

# Cambiar a usuario no-root
USER docutrace

# Exponer puerto
EXPOSE ${PORT}

CMD ["./docutrace-backend"]