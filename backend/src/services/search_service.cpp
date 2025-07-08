#include "services/search_service.hpp"
#include <fstream>
#include <iostream>

namespace DocuTrace::Services
{
    SearchService::SearchService() : engine_(std::make_unique<Infrastructure::BM25Engine>())
    {
    }

    std::vector<Models::SearchResult> SearchService::Search(
        const Models::SearchRequest& request) const
    {
        auto results = engine_->Search(request.query, request.limit);
        std::vector<Models::SearchResult> model_results;
        model_results.reserve(results.size());

        for (const auto& result : results)
        {
            model_results.emplace_back(result.content, result.score, result.document_id);
        }

        return model_results;
    }

    bool SearchService::IndexDocument(const Models::IndexDocumentRequest& request)
    {
        try
        {
            engine_->IndexDocument(request.content);
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error al indexar documento: " << e.what() << std::endl;
            return false;
        }
    }

    size_t SearchService::IndexDocuments(const Models::IndexDocumentsRequest& request)
    {
        size_t indexed_count = 0;
        try
        {
            engine_->IndexDocuments(request.documents);
            indexed_count = request.documents.size();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error al indexar documentos: " << e.what() << std::endl;
        }
        return indexed_count;
    }

    Models::SystemStats SearchService::GetStats() const
    {
        Models::SystemStats stats;
        stats.total_documents = GetDocumentCount();
        // Los otros campos ya tienen valores por defecto
        return stats;
    }

    bool SearchService::ClearIndex()
    {
        try
        {
            engine_->Clear();
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error al limpiar índice: " << e.what() << std::endl;
            return false;
        }
    }

    size_t SearchService::LoadDocumentsFromFile(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            throw std::runtime_error("No se puede abrir el archivo: " + filepath);
        }

        std::vector<std::string> documents;
        std::string line;

        while (std::getline(file, line))
        {
            if (!line.empty())
            {
                documents.push_back(line);
            }
        }

        if (!documents.empty())
        {
            Models::IndexDocumentsRequest request;
            request.documents = std::move(documents);
            return IndexDocuments(request);
        }

        return 0;
    }

    size_t SearchService::GetDocumentCount() const
    {
        return engine_->GetDocumentCount();
    }

} // namespace DocuTrace::Services