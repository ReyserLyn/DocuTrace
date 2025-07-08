#pragma once

#include <memory>
#include "crow/app.h"
#include "crow/middlewares/cors.h"
#include "services/search_service.hpp"

namespace DocuTrace::Controllers
{
    class SearchController
    {
      private:
        std::shared_ptr<Services::SearchService> search_service_;

      public:
        explicit SearchController(std::shared_ptr<Services::SearchService> service);
        ~SearchController() = default;

        // No copyable pero movible
        SearchController(const SearchController&) = delete;
        SearchController& operator=(const SearchController&) = delete;
        SearchController(SearchController&&) = default;
        SearchController& operator=(SearchController&&) = default;

        // Registrar todas las rutas en la app de Crow
        void RegisterRoutes(crow::App<crow::CORSHandler>& app);
    };

} // namespace DocuTrace::Controllers