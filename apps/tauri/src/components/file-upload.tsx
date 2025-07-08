import React, { useRef, useState, useEffect } from "react";
import { Button } from "./ui/button";
import { Card } from "./ui/card";
import { toast } from "sonner";
import { CheckCircle, Upload, X, FileText } from "lucide-react";

// Configuración reutilizada del backend
const MAX_FILE_SIZE = 50 * 1024 * 1024; // 50MB
const SUPPORTED_EXTENSIONS = ".txt,.pdf,.doc,.docx,.md";

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
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const [internalIsUploading, setInternalIsUploading] = useState(false);

  const isUploading = externalIsUploading ?? internalIsUploading;

  useEffect(() => {
    if (uploadSuccess) {
      toast.success("¡Archivo subido exitosamente!", {
        description: selectedFile
          ? `${selectedFile.name} se agregó al índice`
          : undefined,
        icon: <CheckCircle className="h-4 w-4" />,
      });
      setSelectedFile(null);
      if (fileInputRef.current) {
        fileInputRef.current.value = "";
      }
    }
  }, [uploadSuccess, selectedFile]);

  const handleButtonClick = () => {
    fileInputRef.current?.click();
  };

  const handleFileSelect = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      if (file.size > MAX_FILE_SIZE) {
        const maxSizeMB = Math.round(MAX_FILE_SIZE / 1024 / 1024);
        const errorMsg = `El archivo es muy grande. Tamaño máximo: ${maxSizeMB}MB`;
        toast.error("Archivo muy grande", {
          description: errorMsg,
          icon: <X className="h-4 w-4" />,
        });
        return;
      }

      setSelectedFile(file);
      toast.info("Archivo seleccionado", {
        description: `${file.name} listo para subir`,
        icon: <FileText className="h-4 w-4" />,
      });
    }
  };

  const handleUpload = async () => {
    if (!selectedFile) return;

    setInternalIsUploading(true);

    try {
      toast.loading("Subiendo archivo...", {
        description: `Procesando ${selectedFile.name}`,
        icon: <Upload className="h-4 w-4 animate-pulse" />,
        id: "upload-progress",
      });

      await onUpload(selectedFile);

      toast.dismiss("upload-progress");
    } catch (err) {
      toast.dismiss("upload-progress");
      const errorMsg =
        err instanceof Error ? err.message : "Error al subir el archivo";
      toast.error("Error al subir archivo", {
        description: errorMsg,
        icon: <X className="h-4 w-4" />,
      });
    } finally {
      setInternalIsUploading(false);
    }
  };

  const handleCancel = () => {
    setSelectedFile(null);
    if (fileInputRef.current) {
      fileInputRef.current.value = "";
    }
  };

  return (
    <Card
      className={`p-6 border-dashed border-2 transition-all duration-200 ${
        selectedFile
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
          className="hidden"
          disabled={isUploading || disabled}
        />

        {/* Área de selección de archivos */}
        <div className="text-center space-y-3">
          <div
            className={`w-14 h-14 mx-auto rounded-full flex items-center justify-center transition-colors ${
              selectedFile
                ? "bg-primary/10 text-primary"
                : "bg-muted text-muted-foreground"
            }`}
          >
            {selectedFile ? (
              <CheckCircle className="w-7 h-7" />
            ) : (
              <Upload className="w-7 h-7" />
            )}
          </div>
          <div>
            <h3 className="text-lg font-medium">
              {selectedFile ? "Archivo seleccionado" : "Seleccionar archivo"}
            </h3>
            <p className="text-sm text-muted-foreground mt-1">
              {selectedFile ? (
                <span className="flex items-center justify-center gap-1">
                  <FileText className="h-3 w-3" />
                  {selectedFile.name}
                </span>
              ) : (
                `Formatos: ${SUPPORTED_EXTENSIONS.replace(
                  /\./g,
                  ""
                ).toUpperCase()}`
              )}
            </p>
            {selectedFile && (
              <p className="text-xs text-muted-foreground mt-1">
                {(selectedFile.size / 1024 / 1024).toFixed(1)} MB
              </p>
            )}
          </div>
        </div>

        {/* Botones de acción */}
        <div className="flex flex-col space-y-2 w-full max-w-xs">
          {!selectedFile && (
            <Button
              onClick={handleButtonClick}
              disabled={isUploading || disabled}
              className="w-full"
            >
              📁 Examinar archivos
            </Button>
          )}

          {selectedFile && (
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
                    Subir archivo
                  </span>
                )}
              </Button>

              <Button
                onClick={handleCancel}
                disabled={isUploading || disabled}
                variant="outline"
                className="w-full border-red-300 text-red-600 hover:bg-red-50 transition-all"
              >
                <X className="h-4 w-4 mr-2" />
                Cancelar
              </Button>
            </>
          )}
        </div>

        {/* Información adicional */}
        <p className="text-xs text-muted-foreground text-center max-w-xs">
          Tamaño máximo: {Math.round(MAX_FILE_SIZE / 1024 / 1024)}MB
        </p>
      </div>
    </Card>
  );
};
