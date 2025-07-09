import { create } from "zustand";
import { devtools, persist } from "zustand/middleware";
import { toast } from "sonner";

interface AuthState {
  isAuthenticated: boolean;
  adminPassword: string;
  authenticate: (password: string) => boolean;
  logout: () => void;
}

export const useAuthStore = create<AuthState>()(
  devtools(
    persist(
      (set, get) => ({
        isAuthenticated: false,
        adminPassword: import.meta.env.VITE_ADMIN_PASSWORD || "admin",

        authenticate: (password: string): boolean => {
          const { adminPassword } = get();
          if (password === adminPassword) {
            set({ isAuthenticated: true });
            toast.success("Autenticación exitosa", {
              description: "Bienvenido al panel de administración",
            });
            return true;
          } else {
            toast.error("Contraseña incorrecta", {
              description: "Verifica la contraseña e intenta nuevamente",
            });
            return false;
          }
        },

        logout: () => {
          set({ isAuthenticated: false });
          toast.info("Sesión cerrada", {
            description: "Has salido del panel de administración",
          });
        },
      }),
      {
        name: "docutrace-auth",
        partialize: (state) => ({
          isAuthenticated: state.isAuthenticated,
        }),
      }
    ),
    {
      name: "auth-store",
    }
  )
);
