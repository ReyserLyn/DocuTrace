import {
  FileText,
  Home as HomeIcon,
  Settings as SettingsIcon,
} from "lucide-react";
import { Button } from "@/components/ui/button";
import { cn } from "@/lib/utils";
import { useViewStore } from "@/store/useViewStore";
import { Home } from "@/views/Home";
import { Settings } from "@/views/Settings";

function App() {
  const { currentView, setView } = useViewStore();

  const renderView = () => {
    switch (currentView) {
      case "home":
        return <Home />;
      case "settings":
        return <Settings />;
      default:
        return <Home />;
    }
  };

  return (
    <div className="min-h-screen bg-background text-foreground">
      {/* Header con navegación */}
      <header className="border-b bg-card/50 backdrop-blur supports-[backdrop-filter]:bg-card/60">
        <div className="container mx-auto px-4 py-4">
          <div className="flex items-center justify-between">
            {/* Logo y navegación principal */}
            <div className="flex items-center gap-6">
              <div className="flex items-center gap-2">
                <FileText className="h-8 w-8 text-primary" />
                <h1 className="text-2xl font-bold">DocuTrace</h1>
              </div>

              {/* Navegación por tabs */}
              <nav className="hidden md:flex items-center gap-1">
                <Button
                  variant={currentView === "home" ? "default" : "ghost"}
                  size="sm"
                  onClick={() => setView("home")}
                  className={cn(
                    "gap-2",
                    currentView === "home" &&
                      "bg-primary text-primary-foreground"
                  )}
                >
                  <HomeIcon className="h-4 w-4" />
                  Inicio
                </Button>
                <Button
                  variant={currentView === "settings" ? "default" : "ghost"}
                  size="sm"
                  onClick={() => setView("settings")}
                  className={cn(
                    "gap-2",
                    currentView === "settings" &&
                      "bg-primary text-primary-foreground"
                  )}
                >
                  <SettingsIcon className="h-4 w-4" />
                  Configuración
                </Button>
              </nav>
            </div>
          </div>
        </div>
      </header>

      {/* Contenido principal */}
      <main className="min-h-[calc(100vh-80px)]">{renderView()}</main>

      {/* Footer opcional */}
      <footer className="border-t bg-card/30 backdrop-blur supports-[backdrop-filter]:bg-card/50">
        <div className="container mx-auto px-4 py-3">
          <div className="flex items-center justify-between text-sm text-muted-foreground">
            <p>© 2024 DocuTrace - Motor de búsqueda BM25</p>
            <div className="flex items-center gap-4">
              <span>v1.0.0</span>
              <div className="flex items-center gap-1">
                <div className="h-2 w-2 bg-green-500 rounded-full animate-pulse"></div>
                <span>Activo</span>
              </div>
            </div>
          </div>
        </div>
      </footer>
    </div>
  );
}

export default App;
