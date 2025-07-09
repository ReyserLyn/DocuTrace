import { SearchResponse, UploadError, UploadResponse } from "@/interfaces";

class BackendService {
  private baseUrl = import.meta.env.VITE_BACKEND_URL || "http://localhost:8000";

  // Configuración de upload
  private readonly MAX_FILE_SIZE = 50 * 1024 * 1024; // 50MB
  private readonly SUPPORTED_EXTENSIONS = [
    ".txt",
    ".pdf",
    ".docx",
    ".doc",
    ".md",
  ];
  private readonly SUPPORTED_MIME_TYPES = [
    "text/plain",
    "application/pdf",
    "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
    "application/msword",
    "text/markdown",
  ];

  // Verificar estado del backend usando comando de Tauri
  async checkBackendStatus(): Promise<boolean> {
    try {
      const response = await fetch(`${this.baseUrl}/health`);
      if (!response.ok) {
        throw new Error("Backend no está respondiendo");
      }
      return true;
    } catch (error) {
      console.error("Backend status check failed:", error);
      return false;
    }
  }

  // Validar archivo antes del upload
  private validateFile(file: File): UploadError | null {
    if (file.size > this.MAX_FILE_SIZE) {
      return {
        type: "validation",
        message: `Archivo demasiado grande (${(file.size / 1024 / 1024).toFixed(
          1
        )}MB)`,
        details: `El tamaño máximo permitido es ${
          this.MAX_FILE_SIZE / 1024 / 1024
        }MB`,
      };
    }

    // Verificar si el archivo está vacío
    if (file.size === 0) {
      return {
        type: "validation",
        message: "El archivo está vacío",
        details: "Selecciona un archivo que contenga datos",
      };
    }

    // Verificar extensión
    const fileExtension = "." + file.name.split(".").pop()?.toLowerCase();
    if (!this.SUPPORTED_EXTENSIONS.includes(fileExtension)) {
      return {
        type: "validation",
        message: `Tipo de archivo no soportado (${fileExtension})`,
        details: `Tipos soportados: ${this.SUPPORTED_EXTENSIONS.join(", ")}`,
      };
    }

    // Verificar MIME type (si está disponible)
    if (file.type && !this.SUPPORTED_MIME_TYPES.includes(file.type)) {
      console.warn("MIME type no reconocido:", file.type);
    }

    return null;
  }

  // Realizar búsqueda
  async search(query: string, limit: number = 10): Promise<SearchResponse> {
    try {
      const url = `${this.baseUrl}/api/search?query=${encodeURIComponent(
        query
      )}&limit=${limit}`;
      const response = await fetch(url);

      if (!response.ok) {
        let errorMessage = `Error del servidor (${response.status})`;
        try {
          const errorData = await response.json();
          if (errorData.error) {
            errorMessage = errorData.error;
          }
        } catch {}
        throw new Error(errorMessage);
      }

      const data = await response.json();
      return data;
    } catch (error) {
      console.error("Search failed:", error);
      if (error instanceof Error) {
        throw error;
      }
      throw new Error("Error de red al realizar la búsqueda");
    }
  }

  async uploadDocument(file: File): Promise<UploadResponse> {
    const validationError = this.validateFile(file);
    if (validationError) {
      throw new Error(
        validationError.message +
          (validationError.details ? ` - ${validationError.details}` : "")
      );
    }

    try {
      const url = `${this.baseUrl}/api/upload`;

      const formData = new FormData();
      formData.append("file", file, file.name);

      const response = await fetch(url, {
        method: "POST",
        body: formData,
        signal: AbortSignal.timeout(30000),
      });

      if (!response.ok) {
        let errorMessage = `Error del servidor (${response.status})`;
        let errorDetails = "";

        try {
          const errorData = await response.text();

          try {
            const jsonError = JSON.parse(errorData);
            if (jsonError.error) {
              throw new Error(jsonError.error);
            }
          } catch {
            if (errorData.length > 0 && errorData.length < 200) {
              errorDetails = errorData;
            }
          }
        } catch (e) {
          if (e instanceof Error) {
            throw e;
          }
        }

        throw new Error(
          errorMessage + (errorDetails ? ` - ${errorDetails}` : "")
        );
      }

      const result = await response.json();
      console.log("[+] Upload exitoso:", result);
      return result;
    } catch (error) {
      console.error("Upload failed:", error);

      if (error instanceof Error) {
        if (
          error.message.includes("Backend no está respondiendo") ||
          error.message.includes("Error del servidor") ||
          error.message.includes("Formato de archivo") ||
          error.message.includes("demasiado grande")
        ) {
          throw error;
        }

        if (error.name === "AbortError") {
          throw new Error("Timeout: El archivo tardó demasiado en subir");
        }

        if (error.message.includes("fetch")) {
          throw new Error("Error de conexión: No se pudo conectar al backend");
        }
      }

      throw new Error("Error desconocido al subir el archivo");
    }
  }

  // Test de conectividad
  async testConnection(): Promise<boolean> {
    try {
      const response = await fetch(`${this.baseUrl}/health`);
      return response.ok;
    } catch (error) {
      console.error("Connection test failed:", error);
      return false;
    }
  }
}

// Instancia singleton del servicio
export const backendService = new BackendService();
