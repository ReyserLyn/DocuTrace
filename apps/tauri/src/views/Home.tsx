import { useState } from "react";
import {
  Search,
  Loader2,
  AlertCircle,
  CheckCircle,
  FileText,
} from "lucide-react";
import { Button } from "@/components/ui/button";
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { FileUpload } from "@/components/file-upload";
import {
  useSearchMutation,
  useUploadDocument,
  useBackendStatus,
} from "@/hooks/useBackend";
import { SearchResult } from "@/interfaces";

export function Home() {
  const [searchQuery, setSearchQuery] = useState("");

  // TanStack Query hooks
  const { data: backendConnected, isLoading: statusLoading } =
    useBackendStatus();

  const searchMutation = useSearchMutation();
  const uploadMutation = useUploadDocument();

  // Manejar búsqueda
  const handleSearch = async () => {
    if (!searchQuery.trim()) return;

    try {
      await searchMutation.mutateAsync({
        query: searchQuery,
        limit: 10,
      });
    } catch (error) {
      console.error("Error en búsqueda:", error);
    }
  };

  // Manejar upload
  const handleUpload = async (file: File) => {
    try {
      await uploadMutation.mutateAsync(file);
    } catch (error) {
      console.error("Error en upload:", error);
    }
  };

  const searchResults = searchMutation.data?.results || [];
  const isSearching = searchMutation.isPending;

  return (
    <div className="container mx-auto px-4 py-8 grid gap-8 lg:grid-cols-3">
      {/* Panel Principal - Búsqueda */}
      <div className="lg:col-span-2 space-y-6">
        {/* Búsqueda */}
        <Card>
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <Search className="h-5 w-5" />
              Buscar Documentos
            </CardTitle>
            <CardDescription>
              Encuentra documentos usando nuestro motor de búsqueda BM25
              concurrente
            </CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="flex gap-2">
              <Input
                placeholder="Escribe tu consulta de búsqueda..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                onKeyDown={(e) => e.key === "Enter" && handleSearch()}
                className="flex-1"
                disabled={isSearching}
              />
              <Button
                onClick={handleSearch}
                disabled={
                  isSearching || !searchQuery.trim() || !backendConnected
                }
              >
                {isSearching ? (
                  <>
                    <Loader2 className="h-4 w-4 mr-2 animate-spin" />
                    Buscando...
                  </>
                ) : (
                  "Buscar"
                )}
              </Button>
            </div>

            {/* Estado de la conexión del backend */}
            {statusLoading ? (
              <p className="text-sm text-muted-foreground">
                Verificando conexión...
              </p>
            ) : !backendConnected ? (
              <div className="flex items-center gap-2 text-amber-600">
                <AlertCircle className="h-4 w-4" />
                <p className="text-sm">
                  Backend no conectado. Verifica que esté ejecutándose.
                </p>
              </div>
            ) : (
              <div className="flex items-center gap-2 text-green-600">
                <CheckCircle className="h-4 w-4" />
                <p className="text-sm">Backend conectado y listo</p>
              </div>
            )}

            {/* Error de búsqueda */}
            {searchMutation.isError && (
              <div className="p-3 bg-destructive/10 border border-destructive/20 rounded-lg">
                <p className="text-sm text-destructive">
                  Error al realizar la búsqueda: {searchMutation.error?.message}
                </p>
              </div>
            )}

            {searchQuery && !isSearching && (
              <p className="text-sm text-muted-foreground">
                Presiona Enter o haz clic en Buscar para encontrar documentos
                relacionados con "{searchQuery}"
              </p>
            )}
          </CardContent>
        </Card>

        {/* Resultados de Búsqueda */}
        {searchResults.length > 0 && (
          <Card>
            <CardHeader>
              <CardTitle>Resultados de Búsqueda</CardTitle>
              <CardDescription>
                {searchResults.length} documento(s) encontrado(s) para "
                {searchQuery}"
              </CardDescription>
            </CardHeader>
            <CardContent className="space-y-4">
              {searchResults.map((result: SearchResult) => (
                <div
                  key={result.document_id}
                  className="p-4 border rounded-lg hover:bg-muted/50 transition-colors cursor-pointer"
                >
                  <div className="flex items-center justify-between mb-2">
                    <div className="flex items-center gap-2">
                      <FileText className="h-4 w-4 text-muted-foreground" />
                      <span className="text-sm font-medium text-muted-foreground">
                        Documento #{result.document_id}
                      </span>
                    </div>
                    <span className="text-sm bg-primary/10 text-primary px-2 py-1 rounded">
                      Score: {(result.score * 100).toFixed(1)}%
                    </span>
                  </div>
                  <p className="text-sm leading-relaxed">
                    {result.content_preview}
                  </p>
                  <div className="mt-2 flex gap-2">
                    <Button variant="outline" size="sm">
                      Ver completo
                    </Button>
                    <Button variant="ghost" size="sm">
                      Copiar
                    </Button>
                  </div>
                </div>
              ))}
            </CardContent>
          </Card>
        )}

        {/* Estado vacío */}
        {!isSearching &&
          searchResults.length === 0 &&
          !searchMutation.isError && (
            <Card>
              <CardContent className="pt-6">
                <div className="text-center py-12">
                  <Search className="h-12 w-12 mx-auto text-muted-foreground mb-4" />
                  <h3 className="text-lg font-medium mb-2">
                    ¡Comienza a buscar!
                  </h3>
                  <p className="text-muted-foreground mb-4">
                    Escribe una consulta en el buscador para encontrar
                    documentos relevantes
                  </p>
                  <p className="text-sm text-muted-foreground">
                    Sube documentos para empezar a construir tu base de
                    conocimiento
                  </p>
                </div>
              </CardContent>
            </Card>
          )}
      </div>

      {/* Sidebar */}
      <div className="space-y-6">
        {/* Upload mejorado */}
        <Card>
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <FileText className="h-5 w-5" />
              Subir Documento
            </CardTitle>
            <CardDescription>
              Agrega nuevos documentos al índice BM25
            </CardDescription>
          </CardHeader>
          <CardContent>
            <FileUpload
              onUpload={handleUpload}
              isUploading={uploadMutation.isPending}
              uploadSuccess={uploadMutation.isSuccess}
              uploadError={uploadMutation.error?.message || null}
              disabled={!backendConnected}
            />
          </CardContent>
        </Card>
      </div>
    </div>
  );
}
