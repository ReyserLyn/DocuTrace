#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include "infrastructure/bm25_engine.hpp"
#include "models/search_models.hpp"

namespace DocuTrace::Services
{
    class SearchService
    {
      private:
        std::unique_ptr<Infrastructure::BM25Engine> engine_;

        /**
         * @brief Carga documentos existentes desde el índice al inicializar
         */
        void LoadExistingDocuments();

      public:
        SearchService();
        ~SearchService() = default;

        // No copyable pero movible
        SearchService(const SearchService&) = delete;
        SearchService& operator=(const SearchService&) = delete;
        SearchService(SearchService&&) = default;
        SearchService& operator=(SearchService&&) = default;

        /**
         * @brief Realiza una búsqueda en el índice
         * @param request Parámetros de búsqueda validados
         * @return Vector de resultados ordenados por relevancia
         */
        std::vector<Models::SearchResult> Search(const Models::SearchRequest& request) const;

        /**
         * @brief Indexa un documento único
         * @param request Documento a indexar validado
         * @return true si se indexó correctamente
         */
        bool IndexDocument(const Models::IndexDocumentRequest& request);

        /**
         * @brief Indexa múltiples documentos
         * @param request Lista de documentos a indexar validados
         * @return Número de documentos indexados exitosamente
         */
        size_t IndexDocuments(const Models::IndexDocumentsRequest& request);

        /**
         * @brief Obtiene estadísticas del sistema de búsqueda
         * @return Estadísticas actuales del índice
         */
        Models::SystemStats GetStats() const;

        /**
         * @brief Limpia completamente el índice
         * @return true si se limpió correctamente
         */
        bool ClearIndex();

        /**
         * @brief Carga documentos desde archivo
         * @param filepath Ruta al archivo de documentos
         * @return Número de documentos cargados
         * @throws std::runtime_error si no puede abrir el archivo
         */
        size_t LoadDocumentsFromFile(const std::string& filepath);

        /**
         * @brief Obtiene el número total de documentos indexados
         * @return Número de documentos en el índice
         */
        size_t GetDocumentCount() const;
    };

} // namespace DocuTrace::Services