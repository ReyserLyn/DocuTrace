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
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { useSearchMutation, useBackendStatus } from "@/hooks/useBackend";
import { SearchResult } from "@/interfaces";

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
              Motor de búsqueda inteligente con tecnología BM25 para encontrar
              exactamente lo que necesitas en tu biblioteca de documentos
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
      {(searchResults.length > 0 || searchMutation.isError) && (
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

              {/* Resultados */}
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

                  <div className="space-y-4">
                    {searchResults.map((result: SearchResult) => (
                      <Card
                        key={result.document_id}
                        className="hover:shadow-md transition-shadow cursor-pointer"
                      >
                        <CardContent className="p-6">
                          <div className="flex items-start justify-between mb-3">
                            <div className="flex items-center gap-2">
                              <FileText className="h-5 w-5 text-muted-foreground" />
                              <span className="font-medium text-muted-foreground">
                                Documento #{result.document_id}
                              </span>
                            </div>
                            <div className="bg-primary/10 text-primary px-3 py-1 rounded-full text-sm font-medium">
                              {(result.score * 100).toFixed(1)}% relevancia
                            </div>
                          </div>

                          <p className="text-foreground leading-relaxed mb-4">
                            {result.content_preview}
                          </p>

                          <div className="flex gap-2">
                            <Button variant="outline" size="sm">
                              Ver completo
                            </Button>
                            <Button variant="ghost" size="sm">
                              Copiar contenido
                            </Button>
                          </div>
                        </CardContent>
                      </Card>
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
