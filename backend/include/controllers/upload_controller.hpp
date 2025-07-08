#pragma once

#include <memory>
#include "crow/app.h"
#include "crow/middlewares/cors.h"
#include "services/search_service.hpp"

namespace DocuTrace::Controllers
{
    class UploadController
    {
      private:
        std::shared_ptr<Services::SearchService> search_service_;

      public:
        explicit UploadController(std::shared_ptr<Services::SearchService> search_service);

        void RegisterRoutes(crow::App<crow::CORSHandler>& app);
    };

} // namespace DocuTrace::Controllers