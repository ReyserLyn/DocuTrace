#include <ctime>
#include <iostream>
#include <memory>
#include <thread>
#include "controllers/health_controller.hpp"
#include "controllers/search_controller.hpp"
#include "controllers/upload_controller.hpp"
#include "crow/app.h"
#include "services/search_service.hpp"
#include "shared/env_utils.hpp"

void setupCORS(crow::SimpleApp& app)
{
    // Configurar CORS para todas las rutas
    CROW_ROUTE(app, "/").methods("OPTIONS"_method)(
        [](const crow::request& req)
        {
            crow::response res(200);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return res;
        });
}

int main()
{
    try
    {
        DocuTrace::Shared::EnvUtils::LoadEnvFile();

        const auto PORT = std::stoi(DocuTrace::Shared::EnvUtils::GetEnv("PORT", "8000"));

        crow::SimpleApp app;

        // Configurar CORS
        setupCORS(app);

        // Registrar rutas de salud
        auto health_controller = std::make_unique<DocuTrace::Controllers::HealthController>();
        health_controller->RegisterRoutes(app);

        // Crear servicio y controlador de búsqueda
        auto search_service = std::make_shared<DocuTrace::Services::SearchService>();
        auto search_controller =
            std::make_unique<DocuTrace::Controllers::SearchController>(search_service);
        search_controller->RegisterRoutes(app);

        // Crear servicio y controlador de subida
        auto upload_controller =
            std::make_unique<DocuTrace::Controllers::UploadController>(search_service);
        upload_controller->RegisterRoutes(app);

        // Ruta adicional de salud para la API
        CROW_ROUTE(app, "/api/health")
            .methods("GET"_method)(
                [](const crow::request& req)
                {
                    crow::json::wvalue response;
                    response["status"] = "healthy";
                    response["service"] = "DocuTrace Search API";
                    response["version"] = "2.0.0";
                    response["timestamp"] = std::time(nullptr);

                    return crow::response(200, response);
                });

        /*
        // Cargar datos de prueba si existen
        std::string data_file =
            DocuTrace::Shared::EnvUtils::GetEnv("DATA_FILE", "../../data/docs/sample.txt");
        try
        {
            size_t loaded = search_service->LoadDocumentsFromFile(data_file);
            std::cout << "📚 Cargados " << loaded << " documentos de prueba" << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "⚠️  No se pudieron cargar datos de prueba desde " << data_file << ": "
                      << e.what() << std::endl;
            std::cout << "📝 Puedes indexar documentos usando POST /api/upload" << std::endl;
        }
        */

        std::cout << "🚀 DocuTrace Search API iniciado en puerto " << PORT << std::endl;
        std::cout << "📍 Health check: http://localhost:" << PORT << "/health" << std::endl;
        std::cout << "🔍 API Info: http://localhost:" << PORT << "/api/info" << std::endl;
        std::cout << "📊 Stats: http://localhost:" << PORT << "/api/stats" << std::endl;
        std::cout << "🔎 Documentos indexados: " << search_service->GetDocumentCount() << std::endl;

        app.port(PORT).concurrency(std::thread::hardware_concurrency()).run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ Error fatal: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
