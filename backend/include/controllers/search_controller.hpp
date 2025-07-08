#pragma once

#include <memory>
#include "crow/app.h"
#include "services/search_service.hpp"

namespace DocuTrace::Controllers
{
    class SearchController
    {
      private:
        std::shared_ptr<Services::SearchService> search_service_;

        // Métodos auxiliares para manejo de respuestas HTTP
        crow::response CreateErrorResponse(const std::string& message, int code = 400) const;
        crow::response CreateSuccessResponse(
            const std::vector<Models::SearchResult>& results) const;
        bool ValidateSearchRequest(const crow::json::rvalue& request,
                                   std::string& error_message) const;

      public:
        explicit SearchController(std::shared_ptr<Services::SearchService> service);
        ~SearchController() = default;

        // No copyable pero movible
        SearchController(const SearchController&) = delete;
        SearchController& operator=(const SearchController&) = delete;
        SearchController(SearchController&&) = default;
        SearchController& operator=(SearchController&&) = default;

        // Registrar todas las rutas en la app de Crow
        void RegisterRoutes(crow::SimpleApp& app);

        // Handlers de endpoints HTTP
        crow::response HandleSearch(const crow::request& req);
        crow::response HandleIndexDocument(const crow::request& req);
        crow::response HandleIndexDocuments(const crow::request& req);
        crow::response HandleGetStats(const crow::request& req);
        crow::response HandleClearIndex(const crow::request& req);
    };

} // namespace DocuTrace::Controllers