import { useState } from "react";
import {
  Search,
  Loader2,
  AlertCircle,
  CheckCircle,
  FileText,
  Lightbulb,
  Zap,
  Target,
} from "lucide-react";
import { Button } from "@/components/ui/button";
import { Card, CardContent } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { useSearchMutation, useBackendStatus } from "@/hooks/useBackend";
import { SearchResult } from "@/interfaces";
import {
  Dialog,
  DialogContent,
  DialogTitle,
  DialogTrigger,
} from "@/components/ui/dialog";

export function Home() {
  const [searchQuery, setSearchQuery] = useState("");

  // TanStack Query hooks
  const { data: backendConnected, isLoading: statusLoading } =
    useBackendStatus();

  const searchMutation = useSearchMutation();

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

  const searchResults = searchMutation.data?.results || [];
  const isSearching = searchMutation.isPending;

  // Tips de búsqueda
  const searchTips = [
    {
      icon: <Target className="h-4 w-4 text-blue-500" />,
      title: "Búsqueda específica",
      description: "Usa palabras clave específicas para mejores resultados",
    },
    {
      icon: <Zap className="h-4 w-4 text-green-500" />,
      title: "Motor BM25",
      description: "Algoritmo avanzado que entiende relevancia contextual",
    },
    {
      icon: <FileText className="h-4 w-4 text-purple-500" />,
      title: "Múltiples formatos",
      description: "Busca en documentos .txt, .pdf, .doc y más",
    },
  ];

  return (
    <div className="min-h-[calc(100vh-140px)] flex flex-col">
      {/* Hero Section - Buscador Principal */}
      <div className="flex-1 flex items-center justify-center px-4 py-12">
        <div className="w-full max-w-4xl space-y-8">
          {/* Título y descripción */}
          <div className="text-center space-y-4">
            <h1 className="text-4xl font-bold tracking-tight sm:text-5xl lg:text-6xl">
              <span className="text-primary">Busca</span> en tus documentos
            </h1>
            <p className="text-xl text-muted-foreground max-w-2xl mx-auto">
              Motor de búsqueda inteligente basado en <b>índice invertido</b> y{" "}
              <b>BM25</b>, como los grandes buscadores, para encontrar
              exactamente lo que necesitas en tu biblioteca de documentos.
            </p>
          </div>

          {/* Buscador principal */}
          <div className="space-y-4">
            <div className="flex gap-3 max-w-2xl mx-auto">
              <div className="relative flex-1">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-muted-foreground h-5 w-5" />
                <Input
                  placeholder="¿Qué estás buscando?"
                  value={searchQuery}
                  onChange={(e) => setSearchQuery(e.target.value)}
                  onKeyDown={(e) => e.key === "Enter" && handleSearch()}
                  className="pl-10 h-12 text-lg"
                  disabled={isSearching}
                />
              </div>
              <Button
                onClick={handleSearch}
                disabled={
                  isSearching || !searchQuery.trim() || !backendConnected
                }
                size="lg"
                className="h-12 px-8"
              >
                {isSearching ? (
                  <>
                    <Loader2 className="h-5 w-5 mr-2 animate-spin" />
                    Buscando...
                  </>
                ) : (
                  <>
                    <Search className="h-5 w-5 mr-2" />
                    Buscar
                  </>
                )}
              </Button>
            </div>

            {/* Estado de conexión compacto */}
            <div className="flex justify-center">
              {statusLoading ? (
                <p className="text-sm text-muted-foreground">
                  Verificando conexión...
                </p>
              ) : !backendConnected ? (
                <div className="flex items-center gap-2 text-amber-600">
                  <AlertCircle className="h-4 w-4" />
                  <p className="text-sm">Backend desconectado</p>
                </div>
              ) : (
                <div className="flex items-center gap-2 text-green-600">
                  <CheckCircle className="h-4 w-4" />
                  <p className="text-sm">Listo para buscar</p>
                </div>
              )}
            </div>
          </div>

          {/* Tips de búsqueda */}
          {!searchQuery && !isSearching && searchResults.length === 0 && (
            <div className="max-w-3xl mx-auto">
              <div className="text-center mb-6">
                <Lightbulb className="h-6 w-6 mx-auto text-amber-500 mb-2" />
                <h3 className="text-lg font-semibold">Consejos de búsqueda</h3>
              </div>
              <div className="grid gap-4 md:grid-cols-3">
                {searchTips.map((tip, index) => (
                  <Card key={index} className="border-muted">
                    <CardContent className="p-4 text-center">
                      <div className="flex justify-center mb-2">{tip.icon}</div>
                      <h4 className="font-medium text-sm mb-1">{tip.title}</h4>
                      <p className="text-xs text-muted-foreground">
                        {tip.description}
                      </p>
                    </CardContent>
                  </Card>
                ))}
              </div>
            </div>
          )}
        </div>
      </div>

      {/* Resultados de búsqueda */}
      {(searchResults.length > 0 ||
        searchMutation.isError ||
        (searchQuery && !isSearching)) && (
        <div className="border-t bg-muted/30">
          <div className="container mx-auto px-4 py-8">
            <div className="max-w-4xl mx-auto">
              {/* Error de búsqueda */}
              {searchMutation.isError && (
                <Card className="border-destructive/20 bg-destructive/5 mb-6">
                  <CardContent className="p-4">
                    <div className="flex items-center gap-2 text-destructive">
                      <AlertCircle className="h-5 w-5" />
                      <p className="font-medium">
                        Error al realizar la búsqueda
                      </p>
                    </div>
                    <p className="text-sm text-destructive/80 mt-1">
                      {searchMutation.error?.message}
                    </p>
                  </CardContent>
                </Card>
              )}

              {/* Sin resultados */}
              {searchQuery &&
                !isSearching &&
                searchResults.length === 0 &&
                !searchMutation.isError && (
                  <div className="flex flex-col items-center justify-center py-16">
                    <Search className="h-12 w-12 text-muted-foreground mb-4" />
                    <h3 className="text-xl font-semibold mb-2">
                      No se encontraron resultados
                    </h3>
                    <p className="text-muted-foreground mb-4 text-center max-w-md">
                      Intenta con otras palabras clave o revisa la ortografía.
                      Recuerda que el motor utiliza un índice invertido y BM25
                      para encontrar coincidencias relevantes.
                    </p>
                  </div>
                )}

              {/* Resultados tipo Google */}
              {searchResults.length > 0 && (
                <div className="space-y-6">
                  <div className="flex items-center justify-between">
                    <h2 className="text-2xl font-bold">
                      Resultados de búsqueda
                    </h2>
                    <p className="text-muted-foreground">
                      {searchResults.length} documento(s) encontrado(s)
                    </p>
                  </div>

                  <div className="space-y-6">
                    {searchResults.map((result: SearchResult) => (
                      <div
                        key={result.document_id}
                        className="group border-b last:border-b-0 border-muted py-6 px-2 hover:bg-muted/50 transition cursor-pointer rounded-lg"
                      >
                        <div className="flex items-center gap-2 mb-1">
                          <span className="text-xs bg-primary/10 text-primary px-2 py-0.5 rounded-full font-medium">
                            Documento #{result.document_id}
                          </span>
                          {/* Aquí se puede agregar un badge de tipo de archivo en el futuro */}
                        </div>
                        <div className="flex items-center gap-2 mb-2">
                          <span className="text-sm text-muted-foreground">
                            {(result.score * 100).toFixed(1)}% relevancia
                          </span>
                        </div>
                        <div className="mb-2">
                          <span className="text-lg font-semibold text-blue-800 group-hover:underline">
                            {/* Aquí se puede mostrar un título si el backend lo provee */}
                            Documento #{result.document_id}
                          </span>
                        </div>
                        <p className="text-foreground leading-relaxed mb-3 text-base">
                          {result.content_preview}
                        </p>
                        <div className="flex gap-2">
                          <Dialog>
                            <DialogTrigger asChild>
                              <Button variant="outline" size="sm">
                                Ver completo
                              </Button>
                            </DialogTrigger>
                            <DialogContent>
                              <DialogTitle>{`Documento #${result.document_id}`}</DialogTitle>
                              <div className="max-h-[60vh] overflow-y-auto whitespace-pre-line text-base mt-4">
                                {result.content_preview}
                              </div>
                            </DialogContent>
                          </Dialog>
                          <Button
                            variant="ghost"
                            size="sm"
                            onClick={() => {
                              navigator.clipboard.writeText(
                                result.content_preview
                              );
                            }}
                          >
                            Copiar contenido
                          </Button>
                        </div>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
