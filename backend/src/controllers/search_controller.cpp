#include "controllers/search_controller.hpp"
#include "models/search_models.hpp"
#include "services/search_service.hpp"

namespace DocuTrace::Controllers
{
    SearchController::SearchController(std::shared_ptr<Services::SearchService> service)
        : search_service_(std::move(service))
    {
    }

    void SearchController::RegisterRoutes(crow::App<crow::CORSHandler>& app)
    {
        CROW_ROUTE(app, "/api/search")
            .methods("GET"_method)(
                [this](const crow::request& req)
                {
                    auto query = req.url_params.get("query");
                    if (!query)
                    {
                        return crow::response(400,
                                              "{\"error\": \"El parámetro 'query' es requerido\"}");
                    }

                    size_t limit = 10;
                    auto limit_str = req.url_params.get("limit");
                    if (limit_str)
                    {
                        try
                        {
                            limit = std::stoul(limit_str);
                        }
                        catch (const std::exception&)
                        {
                            return crow::response(400,
                                                  "{\"error\": \"Parámetro 'limit' inválido\"}");
                        }
                    }

                    Models::SearchRequest search_req{query, limit};
                    if (!search_req.IsValid())
                    {
                        return crow::response(400,
                                              "{\"error\": \"Parámetros de búsqueda inválidos\"}");
                    }

                    auto results = search_service_->Search(search_req);

                    crow::json::wvalue response;
                    response["total_results"] = results.size();

                    std::vector<crow::json::wvalue> results_json;
                    for (const auto& r : results)
                    {
                        crow::json::wvalue result_item;
                        result_item["document_id"] = r.document_id;
                        result_item["content_preview"] = r.content;
                        result_item["score"] = r.score;
                        results_json.push_back(std::move(result_item));
                    }
                    response["results"] = std::move(results_json);
                    response["success"] = true;

                    return crow::response(200, response);
                });

        // Endpoint de información del API
        CROW_ROUTE(app, "/api/info")
            .methods("GET"_method)(
                [](const crow::request& req)
                {
                    crow::json::wvalue info;
                    info["name"] = "DocuTrace Search API";
                    info["version"] = "2.0.0";
                    info["description"] = "Motor de búsqueda BM25 con API REST";
                    info["endpoints"]["health"] = "GET /health, GET /health";
                    info["endpoints"]["search"] = "GET /api/search?query={terminos}";
                    info["endpoints"]["upload"] = "POST /api/upload/{filename}";

                    return crow::response(200, info);
                });
    }

} // namespace DocuTrace::Controllers