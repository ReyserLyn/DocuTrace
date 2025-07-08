import { useState } from "react";
import {
  Shield,
  Upload,
  FileText,
  Trash2,
  Edit,
  LogOut,
  Lock,
  Eye,
  EyeOff,
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
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogHeader,
  DialogTitle,
} from "@/components/ui/dialog";
import { FileUpload } from "@/components/file-upload";
import { useAuthStore } from "@/store/useAuthStore";
import { useViewStore } from "@/store/useViewStore";
import { useUploadDocument, useBackendStatus } from "@/hooks/useBackend";

export function Admin() {
  const { isAuthenticated, authenticate, logout } = useAuthStore();
  const { setView } = useViewStore();
  const [password, setPassword] = useState("");
  const [showPassword, setShowPassword] = useState(false);
  const [isDialogOpen, setIsDialogOpen] = useState(false);

  const { data: backendConnected } = useBackendStatus();
  const uploadMutation = useUploadDocument();

  const handleAuth = () => {
    const success = authenticate(password);
    if (success) {
      setIsDialogOpen(false);
      setPassword("");
    }
  };

  const handleLogout = () => {
    logout();
    setView("home");
  };

  const handleUpload = async (file: File) => {
    try {
      await uploadMutation.mutateAsync(file);
    } catch (error) {
      console.error("Error en upload:", error);
    }
  };

  if (!isAuthenticated) {
    return (
      <>
        <div className="container mx-auto px-4 py-8">
          <div className="max-w-md mx-auto">
            <Card>
              <CardHeader className="text-center">
                <div className="w-16 h-16 mx-auto mb-4 bg-amber-100 rounded-full flex items-center justify-center">
                  <Shield className="h-8 w-8 text-amber-600" />
                </div>
                <CardTitle className="text-2xl">
                  Área de Administración
                </CardTitle>
                <CardDescription>
                  Esta área requiere autenticación de administrador para acceder
                  a las funciones de gestión de documentos.
                </CardDescription>
              </CardHeader>
              <CardContent>
                <Button
                  onClick={() => setIsDialogOpen(true)}
                  className="w-full"
                  size="lg"
                >
                  <Lock className="h-4 w-4 mr-2" />
                  Ingresar como Administrador
                </Button>
              </CardContent>
            </Card>
          </div>
        </div>

        {/* Dialog de autenticación */}
        <Dialog open={isDialogOpen} onOpenChange={setIsDialogOpen}>
          <DialogContent className="sm:max-w-md">
            <DialogHeader>
              <DialogTitle className="flex items-center gap-2">
                <Shield className="h-5 w-5" />
                Autenticación Requerida
              </DialogTitle>
              <DialogDescription>
                Ingresa la contraseña de administrador para acceder a las
                funciones de gestión.
              </DialogDescription>
            </DialogHeader>
            <div className="space-y-4">
              <div className="space-y-2">
                <label htmlFor="password" className="text-sm font-medium">
                  Contraseña
                </label>
                <div className="relative">
                  <Input
                    id="password"
                    type={showPassword ? "text" : "password"}
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    onKeyDown={(e) => e.key === "Enter" && handleAuth()}
                    placeholder="Ingresa la contraseña"
                    className="pr-10"
                  />
                  <Button
                    type="button"
                    variant="ghost"
                    size="sm"
                    className="absolute right-0 top-0 h-full px-3 py-2 hover:bg-transparent"
                    onClick={() => setShowPassword(!showPassword)}
                  >
                    {showPassword ? (
                      <EyeOff className="h-4 w-4" />
                    ) : (
                      <Eye className="h-4 w-4" />
                    )}
                  </Button>
                </div>
              </div>
              <div className="flex gap-2">
                <Button onClick={handleAuth} className="flex-1">
                  Ingresar
                </Button>
                <Button
                  variant="outline"
                  onClick={() => {
                    setIsDialogOpen(false);
                    setPassword("");
                  }}
                >
                  Cancelar
                </Button>
              </div>
            </div>
          </DialogContent>
        </Dialog>
      </>
    );
  }

  return (
    <div className="container mx-auto px-4 py-8">
      {/* Header de administración */}
      <div className="mb-8">
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-3xl font-bold flex items-center gap-3">
              <Shield className="h-8 w-8 text-primary" />
              Panel de Administración
            </h1>
            <p className="text-muted-foreground mt-2">
              Gestiona documentos y configuración del sistema
            </p>
          </div>
          <Button onClick={handleLogout} variant="outline" className="gap-2">
            <LogOut className="h-4 w-4" />
            Cerrar Sesión
          </Button>
        </div>
      </div>

      <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
        {/* Panel de Subida de Archivos */}
        <div className="md:col-span-2">
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center gap-2">
                <Upload className="h-5 w-5" />
                Subir Documentos
              </CardTitle>
              <CardDescription>
                Agrega nuevos documentos al índice de búsqueda
              </CardDescription>
            </CardHeader>
            <CardContent>
              <FileUpload
                onUpload={handleUpload}
                isUploading={uploadMutation.isPending}
                uploadSuccess={uploadMutation.isSuccess}
                uploadError={uploadMutation.error?.message || null}
                disabled={!backendConnected}
                accept=".txt,.pdf,.doc,.docx,.md"
                maxSize={50 * 1024 * 1024}
              />
            </CardContent>
          </Card>
        </div>

        {/* Panel de Estado del Sistema */}
        <Card>
          <CardHeader>
            <CardTitle className="text-lg">Estado del Sistema</CardTitle>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="flex items-center justify-between">
              <span className="text-sm">Backend</span>
              <div className="flex items-center gap-2">
                <div
                  className={`h-2 w-2 rounded-full ${
                    backendConnected ? "bg-green-500" : "bg-red-500"
                  }`}
                ></div>
                <span className="text-sm">
                  {backendConnected ? "Conectado" : "Desconectado"}
                </span>
              </div>
            </div>

            <div className="flex items-center justify-between">
              <span className="text-sm">Uploads</span>
              <span className="text-sm text-muted-foreground">
                {uploadMutation.isPending ? "En progreso..." : "Listo"}
              </span>
            </div>
          </CardContent>
        </Card>

        {/* Funciones Futuras */}
        <Card className="md:col-span-2 lg:col-span-3">
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <FileText className="h-5 w-5" />
              Gestión de Documentos
            </CardTitle>
            <CardDescription>
              Funciones avanzadas (disponibles en futuras versiones)
            </CardDescription>
          </CardHeader>
          <CardContent>
            <div className="grid gap-4 md:grid-cols-3">
              <Button
                variant="outline"
                disabled
                className="h-20 flex-col gap-2"
              >
                <FileText className="h-6 w-6" />
                <span>Ver Documentos</span>
                <span className="text-xs text-muted-foreground">
                  Próximamente
                </span>
              </Button>
              <Button
                variant="outline"
                disabled
                className="h-20 flex-col gap-2"
              >
                <Edit className="h-6 w-6" />
                <span>Renombrar</span>
                <span className="text-xs text-muted-foreground">
                  Próximamente
                </span>
              </Button>
              <Button
                variant="outline"
                disabled
                className="h-20 flex-col gap-2"
              >
                <Trash2 className="h-6 w-6" />
                <span>Eliminar</span>
                <span className="text-xs text-muted-foreground">
                  Próximamente
                </span>
              </Button>
            </div>
          </CardContent>
        </Card>
      </div>
    </div>
  );
}
