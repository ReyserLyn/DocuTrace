import { useQuery, useMutation } from "@tanstack/react-query";
import { backendService } from "@/services/backend-service";
import { SearchResponse, UploadResponse } from "@/interfaces";

// Query Keys para consistencia
export const queryKeys = {
  search: (query: string, limit: number) =>
    ["backend", "search", { query, limit }] as const,
  backendStatus: ["backend", "status"] as const,
};

// Hook para verificar estado del backend
export function useBackendStatus() {
  return useQuery({
    queryKey: queryKeys.backendStatus,
    queryFn: () => backendService.checkBackendStatus(),
    refetchInterval: 30000,
    retry: true,
  });
}

// Hook para búsquedas
export function useSearch(query: string, limit: number = 10) {
  return useQuery({
    queryKey: queryKeys.search(query, limit),
    queryFn: () => backendService.search(query, limit),
    enabled: !!query.trim(),
    staleTime: 2 * 60 * 1000,
  });
}

// Hook para búsqueda manual
export function useSearchMutation() {
  return useMutation<SearchResponse, Error, { query: string; limit?: number }>({
    mutationFn: ({ query, limit = 10 }) => backendService.search(query, limit),
  });
}

// Hook para subir documentos
export function useUploadDocument() {
  const mutation = useMutation<UploadResponse, Error, File>({
    mutationFn: (file: File) => backendService.uploadDocument(file),
    onSuccess: (data) => {
      console.log("[+] Documento subido exitosamente:", data);
    },
    onError: (error) => {
      console.error("[-] Error al subir documento:", error);
    },
  });

  return mutation;
}

// Hook para test de conexión manual
export function useTestConnection() {
  return useMutation<boolean, Error, void>({
    mutationFn: () => backendService.testConnection(),
  });
}
