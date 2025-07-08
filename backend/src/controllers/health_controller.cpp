#include "controllers/health_controller.hpp"
#include <ctime>

namespace DocuTrace::Controllers
{
    void HealthController::RegisterRoutes(crow::App<crow::CORSHandler>& app)
    {
        CROW_ROUTE(app, "/ping")
            .methods("GET"_method)(
                [](const crow::request& req)
                {
                    crow::json::wvalue response;
                    response["status"] = "ok";
                    response["timestamp"] = std::time(nullptr);
                    return crow::response(200, response);
                });

        CROW_ROUTE(app, "/health")
            .methods("GET"_method)(
                [](const crow::request& req)
                {
                    crow::json::wvalue response;
                    response["status"] = "healthy";
                    response["service"] = "DocuTrace Backend";
                    response["version"] = "2.0.0";
                    response["timestamp"] = std::time(nullptr);
                    return crow::response(200, response);
                });

        CROW_ROUTE(app, "/api/health")
            .methods("GET"_method)(
                [](const crow::request& req)
                {
                    crow::json::wvalue response;
                    response["status"] = "healthy";
                    response["service"] = "DocuTrace Backend API";
                    response["version"] = "2.0.0";
                    response["timestamp"] = std::time(nullptr);
                    return crow::response(200, response);
                });
    }
} // namespace DocuTrace::Controllers
