#include "controllers/search_controller.hpp"
#include <iostream>
#include "models/search_models.hpp"
#include "services/search_service.hpp"

namespace DocuTrace::Controllers
{
    SearchController::SearchController(std::shared_ptr<Services::SearchService> service)
        : search_service_(std::move(service))
    {
    }

    void SearchController::RegisterRoutes(crow::SimpleApp& app)
    {
        // Endpoint de búsqueda
        CROW_ROUTE(app, "/api/search")
            .methods("POST"_method)([this](const crow::request& req) { return HandleSearch(req); });

        // Endpoint para indexar un documento
        CROW_ROUTE(app, "/api/documents")
            .methods("POST"_method)([this](const crow::request& req)
                                    { return HandleIndexDocument(req); });

        // Endpoint para indexar múltiples documentos
        CROW_ROUTE(app, "/api/documents/bulk")
            .methods("POST"_method)([this](const crow::request& req)
                                    { return HandleIndexDocuments(req); });

        // Endpoint para obtener estadísticas
        CROW_ROUTE(app, "/api/stats")
            .methods("GET"_method)([this](const crow::request& req)
                                   { return HandleGetStats(req); });

        // Endpoint para limpiar índice
        CROW_ROUTE(app, "/api/index")
            .methods("DELETE"_method)([this](const crow::request& req)
                                      { return HandleClearIndex(req); });

        // Endpoint de información del API
        CROW_ROUTE(app, "/api/info")
            .methods("GET"_method)(
                [](const crow::request& req)
                {
                    crow::json::wvalue info;
                    info["name"] = "DocuTrace Search API";
                    info["version"] = "2.0.0";
                    info["description"] = "Motor de búsqueda BM25 con API REST";
                    info["endpoints"]["health"] = "GET /health, GET /api/health";
                    info["endpoints"]["search"] = "POST /api/search";
                    info["endpoints"]["index_document"] = "POST /api/documents";
                    info["endpoints"]["index_bulk"] = "POST /api/documents/bulk";
                    info["endpoints"]["stats"] = "GET /api/stats";
                    info["endpoints"]["clear_index"] = "DELETE /api/index";

                    return crow::response(200, info);
                });
    }

    crow::response SearchController::HandleSearch(const crow::request& req)
    {
        try
        {
            // Parsear JSON del request
            auto request_json = crow::json::load(req.body);
            if (!request_json)
            {
                return CreateErrorResponse("JSON inválido", 400);
            }

            // Validar request
            std::string error_message;
            if (!ValidateSearchRequest(request_json, error_message))
            {
                return CreateErrorResponse(error_message, 400);
            }

            // Crear y validar request DTO
            Models::SearchRequest search_request;
            search_request.query = request_json["query"].s();
            if (request_json.has("limit"))
            {
                search_request.limit = request_json["limit"].i();
            }

            if (!search_request.IsValid())
            {
                return CreateErrorResponse("Parámetros de búsqueda inválidos", 400);
            }

            // Realizar búsqueda
            auto results = search_service_->Search(search_request);

            // Crear respuesta exitosa
            return CreateSuccessResponse(results);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error en búsqueda: " << e.what() << std::endl;
            return CreateErrorResponse("Error interno del servidor", 500);
        }
    }

    crow::response SearchController::HandleIndexDocument(const crow::request& req)
    {
        try
        {
            auto request_json = crow::json::load(req.body);
            if (!request_json)
            {
                return CreateErrorResponse("JSON inválido", 400);
            }

            if (!request_json.has("content"))
            {
                return CreateErrorResponse("Campo 'content' requerido", 400);
            }

            // Crear y validar request DTO
            Models::IndexDocumentRequest index_request;
            index_request.content = request_json["content"].s();

            if (!index_request.IsValid())
            {
                return CreateErrorResponse("El contenido no puede estar vacío", 400);
            }

            // Indexar documento
            if (!search_service_->IndexDocument(index_request))
            {
                return CreateErrorResponse("Error al indexar documento", 500);
            }

            // Respuesta exitosa
            crow::json::wvalue response;
            response["success"] = true;
            response["message"] = "Documento indexado correctamente";
            response["total_documents"] = search_service_->GetDocumentCount();

            return crow::response(201, response);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error al indexar documento: " << e.what() << std::endl;
            return CreateErrorResponse("Error interno del servidor", 500);
        }
    }

    crow::response SearchController::HandleIndexDocuments(const crow::request& req)
    {
        try
        {
            auto request_json = crow::json::load(req.body);
            if (!request_json)
            {
                return CreateErrorResponse("JSON inválido", 400);
            }

            if (!request_json.has("documents"))
            {
                return CreateErrorResponse("Campo 'documents' requerido", 400);
            }

            // Crear y validar request DTO
            Models::IndexDocumentsRequest index_request;
            for (size_t i = 0; i < request_json["documents"].size(); ++i)
            {
                index_request.documents.push_back(request_json["documents"][i].s());
            }

            if (!index_request.IsValid())
            {
                return CreateErrorResponse(
                    "La lista de documentos no puede estar vacía o contener documentos vacíos",
                    400);
            }

            // Indexar documentos
            size_t indexed_count = search_service_->IndexDocuments(index_request);

            // Respuesta exitosa
            crow::json::wvalue response;
            response["success"] = true;
            response["message"] = "Documentos indexados correctamente";
            response["indexed_count"] = indexed_count;
            response["total_documents"] = search_service_->GetDocumentCount();

            return crow::response(201, response);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error al indexar documentos: " << e.what() << std::endl;
            return CreateErrorResponse("Error interno del servidor", 500);
        }
    }

    crow::response SearchController::HandleGetStats(const crow::request& req)
    {
        try
        {
            auto stats = search_service_->GetStats();

            crow::json::wvalue response;
            response["success"] = true;
            response["stats"]["total_documents"] = stats.total_documents;
            response["stats"]["engine_type"] = stats.engine_type;
            response["stats"]["version"] = stats.version;

            return crow::response(200, response);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error al obtener estadísticas: " << e.what() << std::endl;
            return CreateErrorResponse("Error interno del servidor", 500);
        }
    }

    crow::response SearchController::HandleClearIndex(const crow::request& req)
    {
        try
        {
            if (!search_service_->ClearIndex())
            {
                return CreateErrorResponse("Error al limpiar índice", 500);
            }

            crow::json::wvalue response;
            response["success"] = true;
            response["message"] = "Índice limpiado correctamente";
            response["total_documents"] = 0;

            return crow::response(200, response);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error al limpiar índice: " << e.what() << std::endl;
            return CreateErrorResponse("Error interno del servidor", 500);
        }
    }

    crow::response SearchController::CreateErrorResponse(const std::string& message, int code) const
    {
        crow::json::wvalue response;
        response["success"] = false;
        response["error"]["message"] = message;
        response["error"]["code"] = code;

        return crow::response(code, response);
    }

    crow::response SearchController::CreateSuccessResponse(
        const std::vector<Models::SearchResult>& results) const
    {
        crow::json::wvalue response;
        crow::json::wvalue json_results(crow::json::type::List);

        for (size_t i = 0; i < results.size(); ++i)
        {
            crow::json::wvalue result;
            result["content"] = results[i].content;
            result["score"] = results[i].score;
            result["document_id"] = results[i].document_id;
            json_results[i] = std::move(result);
        }

        response["success"] = true;
        response["results"] = std::move(json_results);
        response["total_results"] = results.size();

        return crow::response(200, response);
    }

    bool SearchController::ValidateSearchRequest(const crow::json::rvalue& request,
                                                 std::string& error_message) const
    {
        if (!request.has("query"))
        {
            error_message = "Campo 'query' requerido";
            return false;
        }

        std::string query = request["query"].s();
        if (query.empty())
        {
            error_message = "La query no puede estar vacía";
            return false;
        }

        if (request.has("limit"))
        {
            try
            {
                int limit = request["limit"].i();
                if (limit <= 0)
                {
                    error_message = "Campo 'limit' debe ser número entero positivo";
                    return false;
                }
            }
            catch (const std::exception&)
            {
                error_message = "Campo 'limit' debe ser número entero positivo";
                return false;
            }
        }

        return true;
    }

} // namespace DocuTrace::Controllers