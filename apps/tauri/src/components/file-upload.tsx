import React, { useRef, useState, useEffect } from "react";
import { Button } from "./ui/button";
import { Card } from "./ui/card";

interface FileUploadProps {
  onUpload: (file: File) => Promise<void>;
  accept?: string;
  maxSize?: number;
  className?: string;
  // Props opcionales para estado externo
  isUploading?: boolean;
  uploadSuccess?: boolean;
  uploadError?: string | null;
  disabled?: boolean;
}

export const FileUpload: React.FC<FileUploadProps> = ({
  onUpload,
  accept = ".txt,.pdf,.doc,.docx",
  maxSize = 10 * 1024 * 1024, // 10MB
  className = "",
  isUploading: externalIsUploading,
  uploadSuccess,
  uploadError,
  disabled = false,
}) => {
  const fileInputRef = useRef<HTMLInputElement>(null);
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const [internalIsUploading, setInternalIsUploading] = useState(false);
  const [internalError, setInternalError] = useState<string | null>(null);

  // Usar estado externo si está disponible, sino el interno
  const isUploading = externalIsUploading ?? internalIsUploading;
  const error = uploadError ?? internalError;

  // Limpiar archivo seleccionado cuando el upload externo sea exitoso
  useEffect(() => {
    if (uploadSuccess) {
      setSelectedFile(null);
      setInternalError(null);
      if (fileInputRef.current) {
        fileInputRef.current.value = "";
      }
    }
  }, [uploadSuccess]);

  const handleButtonClick = () => {
    fileInputRef.current?.click();
  };

  const handleFileSelect = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      setInternalError(null);

      if (file.size > maxSize) {
        setInternalError(
          `El archivo es muy grande. Tamaño máximo: ${maxSize / 1024 / 1024}MB`
        );
        return;
      }

      setSelectedFile(file);
    }
  };

  const handleUpload = async () => {
    if (!selectedFile) return;

    setInternalIsUploading(true);
    setInternalError(null);

    try {
      await onUpload(selectedFile);
      // Mantener el archivo seleccionado después de la subida exitosa
      // para que el usuario vea que se subió correctamente
    } catch (err) {
      setInternalError(
        err instanceof Error ? err.message : "Error al subir el archivo"
      );
    } finally {
      setInternalIsUploading(false);
    }
  };

  const handleCancel = () => {
    setSelectedFile(null);
    setInternalError(null);
    if (fileInputRef.current) {
      fileInputRef.current.value = "";
    }
  };

  return (
    <Card
      className={`p-6 border-dashed border-2 border-gray-300 hover:border-gray-400 transition-colors ${className}`}
    >
      <div className="flex flex-col items-center justify-center space-y-4">
        <input
          type="file"
          ref={fileInputRef}
          onChange={handleFileSelect}
          accept={accept}
          className="hidden"
          disabled={isUploading || disabled}
        />

        {/* Área de selección de archivos */}
        <div className="text-center space-y-2">
          <div className="w-12 h-12 mx-auto bg-gray-100 rounded-full flex items-center justify-center">
            <svg
              className="w-6 h-6 text-gray-400"
              fill="none"
              stroke="currentColor"
              viewBox="0 0 24 24"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                strokeWidth={2}
                d="M7 16a4 4 0 01-.88-7.903A5 5 0 1115.9 6L16 6a5 5 0 011 9.9M15 13l-3-3m0 0l-3 3m3-3v12"
              />
            </svg>
          </div>
          <h3 className="text-lg font-medium text-gray-900">
            {selectedFile ? "Archivo seleccionado" : "Seleccionar archivo"}
          </h3>
          <p className="text-sm text-gray-500">
            {selectedFile
              ? selectedFile.name
              : `Formatos soportados: ${accept}`}
          </p>
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
                className="w-full bg-green-600 hover:bg-green-700 text-white"
              >
                {isUploading ? (
                  <span className="flex items-center justify-center">
                    <svg
                      className="animate-spin -ml-1 mr-2 h-4 w-4 text-white"
                      xmlns="http://www.w3.org/2000/svg"
                      fill="none"
                      viewBox="0 0 24 24"
                    >
                      <circle
                        className="opacity-25"
                        cx="12"
                        cy="12"
                        r="10"
                        stroke="currentColor"
                        strokeWidth="4"
                      ></circle>
                      <path
                        className="opacity-75"
                        fill="currentColor"
                        d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"
                      ></path>
                    </svg>
                    Subiendo...
                  </span>
                ) : (
                  "⬆️ Subir archivo"
                )}
              </Button>

              <Button
                onClick={handleCancel}
                disabled={isUploading || disabled}
                variant="outline"
                className="w-full border-red-300 text-red-600 hover:bg-red-50"
              >
                ❌ Cancelar
              </Button>
            </>
          )}
        </div>

        {/* Estado de error */}
        {error && (
          <div className="w-full max-w-xs p-3 bg-red-50 border border-red-200 rounded-md">
            <p className="text-sm text-red-600 text-center">{error}</p>
          </div>
        )}

        {/* Información adicional */}
        <p className="text-xs text-gray-400 text-center max-w-xs">
          Tamaño máximo: {Math.round(maxSize / 1024 / 1024)}MB
        </p>
      </div>
    </Card>
  );
};
