import { useState } from "react";
import {
  Settings as SettingsIcon,
  Server,
  Search,
  Globe,
  Shield,
  Loader2,
  CheckCircle,
  AlertCircle,
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
import { useTestConnection, useBackendStatus } from "@/hooks/useBackend";

export function Settings() {
  const baseUrl = import.meta.env.VITE_BACKEND_URL || "http://localhost:8000";
  const [backendUrl, setBackendUrl] = useState(baseUrl);
  const [searchLimit, setSearchLimit] = useState(10);
  const [autoConnect, setAutoConnect] = useState(true);

  // TanStack Query hooks
  const { data: backendConnected } = useBackendStatus();
  const testConnectionMutation = useTestConnection();

  const handleTestConnection = async () => {
    testConnectionMutation.mutate();
  };

  return (
    <div className="container mx-auto px-4 py-8 max-w-4xl">
      <div className="space-y-6">
        {/* Header */}
        <div className="flex items-center gap-3 mb-8">
          <SettingsIcon className="h-8 w-8" />
          <div>
            <h1 className="text-3xl font-bold">Configuración</h1>
            <p className="text-muted-foreground">
              Personaliza DocuTrace según tus preferencias
            </p>
          </div>
        </div>

        <div className="grid gap-6 lg:grid-cols-2">
          {/* Conexión al Backend */}
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center gap-2">
                <Server className="h-5 w-5" />
                Backend
              </CardTitle>
              <CardDescription>
                Configura la conexión al servidor DocuTrace
              </CardDescription>
            </CardHeader>
            <CardContent className="space-y-4">
              <div>
                <label className="text-sm font-medium mb-2 block">
                  URL del Backend
                </label>
                <div className="flex gap-2">
                  <Input
                    value={backendUrl}
                    onChange={(e) => setBackendUrl(e.target.value)}
                    placeholder="http://localhost:8000"
                    className="flex-1"
                  />
                  <Button
                    onClick={handleTestConnection}
                    variant="outline"
                    size="sm"
                    disabled={testConnectionMutation.isPending}
                  >
                    {testConnectionMutation.isPending ? (
                      <Loader2 className="h-4 w-4 mr-2 animate-spin" />
                    ) : (
                      <Globe className="h-4 w-4 mr-2" />
                    )}
                    Probar
                  </Button>
                </div>

                {/* Estado de la prueba de conexión */}
                {testConnectionMutation.isSuccess && (
                  <div className="flex items-center gap-2 mt-2 text-green-600">
                    <CheckCircle className="h-4 w-4" />
                    <span className="text-sm">Conexión exitosa</span>
                  </div>
                )}

                {testConnectionMutation.isError && (
                  <div className="flex items-center gap-2 mt-2 text-red-600">
                    <AlertCircle className="h-4 w-4" />
                    <span className="text-sm">Error de conexión</span>
                  </div>
                )}
              </div>

              <div className="flex items-center justify-between">
                <div className="space-y-0.5">
                  <label className="text-sm font-medium">
                    Conexión automática
                  </label>
                  <p className="text-xs text-muted-foreground">
                    Conectar al iniciar la app
                  </p>
                </div>
                <Button
                  variant={autoConnect ? "default" : "outline"}
                  size="sm"
                  onClick={() => setAutoConnect(!autoConnect)}
                >
                  {autoConnect ? "Activado" : "Desactivado"}
                </Button>
              </div>

              {/* Estado actual del backend */}
              <div className="p-3 bg-muted rounded-lg">
                <div className="flex items-center justify-between">
                  <span className="text-sm font-medium">Estado actual:</span>
                  <div className="flex items-center gap-2">
                    {backendConnected ? (
                      <>
                        <div className="h-2 w-2 bg-green-500 rounded-full animate-pulse"></div>
                        <span className="text-sm text-green-600">
                          Conectado
                        </span>
                      </>
                    ) : (
                      <>
                        <div className="h-2 w-2 bg-red-500 rounded-full"></div>
                        <span className="text-sm text-red-600">
                          Desconectado
                        </span>
                      </>
                    )}
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>

          {/* Configuración de Búsqueda */}
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center gap-2">
                <Search className="h-5 w-5" />
                Búsqueda
              </CardTitle>
              <CardDescription>
                Ajusta las preferencias del motor de búsqueda BM25
              </CardDescription>
            </CardHeader>
            <CardContent className="space-y-4">
              <div>
                <label className="text-sm font-medium mb-2 block">
                  Límite de resultados por defecto
                </label>
                <Input
                  type="number"
                  value={searchLimit}
                  onChange={(e) =>
                    setSearchLimit(parseInt(e.target.value) || 10)
                  }
                  min="1"
                  max="100"
                  className="w-24"
                />
                <p className="text-xs text-muted-foreground mt-1">
                  Número máximo de documentos a mostrar (1-100)
                </p>
              </div>
            </CardContent>
          </Card>
        </div>

        {/* Acciones */}
        <div className="flex gap-4 pt-4">
          <Button className="flex-1">
            <Shield className="h-4 w-4 mr-2" />
            Guardar configuración
          </Button>
          <Button variant="outline" className="flex-1">
            Restaurar valores por defecto
          </Button>
        </div>
      </div>
    </div>
  );
}
