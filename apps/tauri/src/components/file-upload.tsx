import React, { useRef, useState, useEffect } from "react";
import { Button } from "./ui/button";
import { Card } from "./ui/card";
import { toast } from "sonner";
import { CheckCircle, Upload, X, FileText } from "lucide-react";

// Configuración reutilizada del backend
const MAX_FILE_SIZE = 50 * 1024 * 1024; // 50MB
const SUPPORTED_EXTENSIONS = ".txt,.pdf,.doc,.docx,.md";
const MAX_FILES = 100;

const formatFileSize = (bytes: number): string => {
  if (bytes === 0) return "0 B";

  if (bytes < 1024) {
    return `${bytes} B`;
  } else if (bytes < 1024 * 1024) {
    return `${(bytes / 1024).toFixed(1)} KB`;
  } else {
    return `${(bytes / 1024 / 1024).toFixed(1)} MB`;
  }
};

interface FileUploadProps {
  onUpload: (file: File) => Promise<void>;
  className?: string;
  isUploading?: boolean;
  uploadSuccess?: boolean;
  disabled?: boolean;
}

export const FileUpload: React.FC<FileUploadProps> = ({
  onUpload,
  className = "",
  isUploading: externalIsUploading,
  uploadSuccess,
  disabled = false,
}) => {
  const fileInputRef = useRef<HTMLInputElement>(null);
  const [selectedFiles, setSelectedFiles] = useState<File[]>([]);
  const [internalIsUploading, setInternalIsUploading] = useState(false);
  const [uploadProgress, setUploadProgress] = useState<{
    [key: string]: "pending" | "uploading" | "success" | "error";
  }>({});

  const isUploading = externalIsUploading ?? internalIsUploading;

  useEffect(() => {
    if (uploadSuccess) {
      setSelectedFiles([]);
      setUploadProgress({});
      if (fileInputRef.current) {
        fileInputRef.current.value = "";
      }
    }
  }, [uploadSuccess]);

  const handleButtonClick = () => {
    fileInputRef.current?.click();
  };

  const handleFileSelect = (event: React.ChangeEvent<HTMLInputElement>) => {
    const files = Array.from(event.target.files || []);
    if (files.length === 0) return;

    if (files.length > MAX_FILES) {
      toast.error("Demasiados archivos", {
        description: `Máximo ${MAX_FILES} archivos permitidos`,
        icon: <X className="h-4 w-4" />,
      });
      return;
    }

    // Validar cada archivo
    const validFiles: File[] = [];
    let hasErrors = false;

    for (const file of files) {
      if (file.size > MAX_FILE_SIZE) {
        const maxSizeMB = Math.round(MAX_FILE_SIZE / 1024 / 1024);
        toast.error(`${file.name} es muy grande`, {
          description: `Tamaño máximo: ${maxSizeMB}MB`,
          icon: <X className="h-4 w-4" />,
        });
        hasErrors = true;
        continue;
      }
      validFiles.push(file);
    }

    if (validFiles.length > 0) {
      setSelectedFiles(validFiles);
      // Inicializar progreso
      const progress: {
        [key: string]: "pending" | "uploading" | "success" | "error";
      } = {};
      validFiles.forEach((file) => {
        progress[file.name] = "pending";
      });
      setUploadProgress(progress);

      toast.info(`${validFiles.length} archivo(s) seleccionado(s)`, {
        description: hasErrors
          ? "Algunos archivos fueron omitidos"
          : "Listos para subir",
        icon: <FileText className="h-4 w-4" />,
      });
    }
  };

  const handleUpload = async () => {
    if (selectedFiles.length === 0) return;

    setInternalIsUploading(true);

    toast.loading("Subiendo archivos...", {
      description: `Procesando ${selectedFiles.length} archivo(s)`,
      icon: <Upload className="h-4 w-4 animate-pulse" />,
      id: "upload-progress",
    });

    // Subir archivos en paralelo
    const uploadPromises = selectedFiles.map(async (file) => {
      try {
        // Actualizar estado a "uploading"
        setUploadProgress((prev) => ({
          ...prev,
          [file.name]: "uploading",
        }));

        await onUpload(file);

        // Actualizar estado a "success"
        setUploadProgress((prev) => ({
          ...prev,
          [file.name]: "success",
        }));

        toast.success(`${file.name} subido`, {
          description: "Agregado al índice exitosamente",
          icon: <CheckCircle className="h-4 w-4" />,
        });

        return { file, success: true };
      } catch (err) {
        // Actualizar estado a "error"
        setUploadProgress((prev) => ({
          ...prev,
          [file.name]: "error",
        }));

        const errorMsg = err instanceof Error ? err.message : "Error al subir";
        toast.error(`Error: ${file.name}`, {
          description: errorMsg,
          icon: <X className="h-4 w-4" />,
        });

        return { file, success: false, error: errorMsg };
      }
    });

    try {
      const results = await Promise.allSettled(uploadPromises);
      const successful = results.filter(
        (result) => result.status === "fulfilled" && result.value.success
      ).length;

      toast.dismiss("upload-progress");

      if (successful === selectedFiles.length) {
        toast.success("¡Todos los archivos subidos!", {
          description: `${successful} archivo(s) procesados exitosamente`,
          icon: <CheckCircle className="h-4 w-4" />,
        });
      } else {
        toast.warning("Upload parcialmente exitoso", {
          description: `${successful}/${selectedFiles.length} archivos subidos`,
        });
      }
    } catch (err) {
      toast.dismiss("upload-progress");
      toast.error("Error general", {
        description: "Error inesperado durante el upload",
        icon: <X className="h-4 w-4" />,
      });
    } finally {
      setInternalIsUploading(false);
    }
  };

  const handleCancel = () => {
    setSelectedFiles([]);
    setUploadProgress({});
    if (fileInputRef.current) {
      fileInputRef.current.value = "";
    }
  };

  const removeFile = (fileToRemove: File) => {
    setSelectedFiles((prev) => prev.filter((file) => file !== fileToRemove));
    setUploadProgress((prev) => {
      const newProgress = { ...prev };
      delete newProgress[fileToRemove.name];
      return newProgress;
    });

    toast.info("Archivo eliminado", {
      description: `${fileToRemove.name} removido de la lista`,
      icon: <X className="h-4 w-4" />,
    });
  };

  return (
    <Card
      className={`p-6 border-dashed border-2 transition-all duration-200 ${
        selectedFiles.length > 0
          ? "border-primary bg-primary/5"
          : "border-muted-foreground/30 hover:border-primary/50 hover:bg-muted/20"
      } ${className}`}
    >
      <div className="flex flex-col items-center justify-center space-y-4">
        <input
          type="file"
          ref={fileInputRef}
          onChange={handleFileSelect}
          accept={SUPPORTED_EXTENSIONS}
          multiple
          className="hidden"
          disabled={isUploading || disabled}
        />

        {/* Área de selección de archivos */}
        <div className="text-center space-y-3">
          <div
            className={`w-14 h-14 mx-auto rounded-full flex items-center justify-center transition-colors ${
              selectedFiles.length > 0
                ? "bg-primary/10 text-primary"
                : "bg-muted text-muted-foreground"
            }`}
          >
            {selectedFiles.length > 0 ? (
              <CheckCircle className="w-7 h-7" />
            ) : (
              <Upload className="w-7 h-7" />
            )}
          </div>
          <div>
            <h3 className="text-lg font-medium">
              {selectedFiles.length > 0
                ? `${selectedFiles.length} archivo(s) seleccionado(s)`
                : "Seleccionar archivos"}
            </h3>
            <p className="text-sm text-muted-foreground mt-1">
              {selectedFiles.length > 0
                ? `Hasta ${MAX_FILES} archivos`
                : `Formatos: ${SUPPORTED_EXTENSIONS.replace(
                    /\./g,
                    ""
                  ).toUpperCase()}`}
            </p>
          </div>
        </div>

        {/* Lista de archivos seleccionados */}
        {selectedFiles.length > 0 && (
          <div className="w-full max-w-md space-y-2 max-h-32 overflow-y-auto">
            {selectedFiles.map((file) => {
              const status = uploadProgress[file.name] || "pending";
              const canRemove = status === "pending" || status === "error";

              return (
                <div
                  key={file.name}
                  className="flex items-center justify-between p-2 bg-muted/50 rounded text-sm group hover:bg-muted/70 transition-colors"
                >
                  <div className="flex items-center gap-2 flex-1 min-w-0">
                    <FileText className="h-3 w-3 flex-shrink-0" />
                    <span className="truncate">{file.name}</span>
                    <span className="text-xs text-muted-foreground flex-shrink-0">
                      ({formatFileSize(file.size)})
                    </span>
                  </div>
                  <div className="flex items-center gap-2 flex-shrink-0 ml-2">
                    {/* Estado del archivo */}
                    <div className="flex-shrink-0">
                      {status === "pending" && (
                        <div className="w-2 h-2 bg-muted-foreground rounded-full"></div>
                      )}
                      {status === "uploading" && (
                        <Upload className="h-3 w-3 animate-pulse text-blue-500" />
                      )}
                      {status === "success" && (
                        <CheckCircle className="h-3 w-3 text-green-500" />
                      )}
                      {status === "error" && (
                        <X className="h-3 w-3 text-red-500" />
                      )}
                    </div>

                    {/* Botón de eliminar individual */}
                    {canRemove && (
                      <Button
                        size="sm"
                        variant="ghost"
                        className="h-5 w-5 p-0 opacity-0 group-hover:opacity-100 transition-opacity hover:bg-destructive/20 hover:text-destructive"
                        onClick={() => removeFile(file)}
                        disabled={isUploading}
                      >
                        <X className="h-3 w-3" />
                      </Button>
                    )}
                  </div>
                </div>
              );
            })}
          </div>
        )}

        {/* Botones de acción */}
        <div className="flex flex-col space-y-2 w-full max-w-xs">
          {selectedFiles.length === 0 && (
            <Button
              onClick={handleButtonClick}
              disabled={isUploading || disabled}
              className="w-full"
            >
              📁 Seleccionar archivos
            </Button>
          )}

          {selectedFiles.length > 0 && (
            <>
              <Button
                onClick={handleUpload}
                disabled={isUploading || disabled}
                className="w-full bg-green-600 hover:bg-green-700 text-white shadow-md hover:shadow-lg transition-all"
              >
                {isUploading ? (
                  <span className="flex items-center justify-center gap-2">
                    <Upload className="h-4 w-4 animate-pulse" />
                    Subiendo...
                  </span>
                ) : (
                  <span className="flex items-center justify-center gap-2">
                    <Upload className="h-4 w-4" />
                    Subir {selectedFiles.length} archivo(s)
                  </span>
                )}
              </Button>

              <div className="flex gap-2">
                <Button
                  onClick={handleButtonClick}
                  disabled={isUploading || disabled}
                  variant="outline"
                  className="flex-1"
                >
                  📁 Agregar más
                </Button>
                <Button
                  onClick={handleCancel}
                  disabled={isUploading || disabled}
                  variant="outline"
                  className="flex-1 border-red-300 text-red-600 hover:bg-red-50 transition-all"
                >
                  <X className="h-4 w-4 mr-1" />
                  Limpiar
                </Button>
              </div>
            </>
          )}
        </div>

        {/* Información adicional */}
        <p className="text-xs text-muted-foreground text-center max-w-xs">
          Máximo {MAX_FILES} archivos •{" "}
          {Math.round(MAX_FILE_SIZE / 1024 / 1024)}MB cada uno
        </p>
      </div>
    </Card>
  );
};
