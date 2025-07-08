#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace DocuTrace::Models
{
    struct SearchResult
    {
        std::string content;
        double score;
        int document_id;

        SearchResult(const std::string& content, double score, int doc_id)
            : content(content), score(score), document_id(doc_id)
        {
        }
    };

    /**
     * @brief DTO para request de búsqueda
     * @note Data Transfer Object para comunicación HTTP
     */
    struct SearchRequest
    {
        std::string query;
        size_t limit = 10;

        bool IsValid() const
        {
            return !query.empty() && limit > 0 && limit <= 100;
        }
    };

    /**
     * @brief DTO para request de indexación de documento único
     */
    struct IndexDocumentRequest
    {
        size_t document_id;
        std::string content;

        bool IsValid() const
        {
            return !content.empty();
        }
    };

    /**
     * @brief DTO para request de indexación masiva
     */
    struct IndexDocumentsRequest
    {
        std::vector<std::string> documents;

        bool IsValid() const
        {
            return !documents.empty() &&
                   std::all_of(documents.begin(), documents.end(),
                               [](const std::string& doc) { return !doc.empty(); });
        }
    };

    /**
     * @brief DTO para respuesta de estadísticas del sistema
     */
    struct SystemStats
    {
        size_t total_documents = 0;
        std::string engine_type = "BM25";
        std::string version = "2.0.0";
    };

    /**
     * @brief DTO genérico para respuestas de API
     */
    template <typename T>
    struct ApiResponse
    {
        bool success;
        T data;
        std::string message;
        int code = 200;

        ApiResponse(bool success, const T& data, const std::string& message = "", int code = 200)
            : success(success), data(data), message(message), code(code)
        {
        }
    };

    /**
     * @brief DTO para respuestas de error
     */
    struct ErrorResponse
    {
        std::string message;
        int code;

        ErrorResponse(const std::string& message, int code = 400) : message(message), code(code)
        {
        }
    };

} // namespace DocuTrace::Models