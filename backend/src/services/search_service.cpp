#include "services/search_service.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>

namespace DocuTrace::Services
{
    // Función helper para obtener el directorio de datos del sistema
    std::filesystem::path get_system_data_dir()
    {
        std::filesystem::path data_dir;

#ifdef _WIN32
        // Windows: %APPDATA%\DocuTrace
        const char* appdata = std::getenv("APPDATA");
        if (appdata)
        {
            data_dir = std::filesystem::path(appdata) / "DocuTrace";
        }
        else
        {
            const char* userprofile = std::getenv("USERPROFILE");
            if (userprofile)
            {
                data_dir = std::filesystem::path(userprofile) / "AppData" / "Roaming" / "DocuTrace";
            }
            else
            {
                data_dir = std::filesystem::current_path() / "data";
            }
        }
#elif __APPLE__
        // macOS: ~/Library/Application Support/DocuTrace
        const char* home = std::getenv("HOME");
        if (home)
        {
            data_dir =
                std::filesystem::path(home) / "Library" / "Application Support" / "DocuTrace";
        }
        else
        {
            data_dir = std::filesystem::current_path() / "data";
        }
#else
        // Linux: ~/.local/share/DocuTrace
        const char* home = std::getenv("HOME");
        if (home)
        {
            data_dir = std::filesystem::path(home) / ".local" / "share" / "DocuTrace";
        }
        else
        {
            data_dir = std::filesystem::current_path() / "data";
        }
#endif

        return data_dir;
    }

    SearchService::SearchService() : engine_(std::make_unique<Infrastructure::BM25Engine>())
    {
        // Cargar documentos existentes al inicializar
        LoadExistingDocuments();
    }

    void SearchService::LoadExistingDocuments()
    {
        try
        {
            auto data_dir = get_system_data_dir();
            auto index_file = data_dir / "document_index.json";

            if (!std::filesystem::exists(index_file))
            {
                return;
            }

            std::ifstream file(index_file);
            if (!file.is_open())
            {
                return;
            }

            nlohmann::json index;
            file >> index;

            if (!index.is_array())
            {
                return;
            }

            size_t loaded_count = 0;
            for (const auto& doc : index)
            {
                if (doc.contains("id") && doc.contains("path"))
                {
                    try
                    {
                        std::string file_path = doc["path"];

                        // Leer contenido del archivo
                        std::ifstream doc_file(file_path);
                        if (doc_file.is_open())
                        {
                            std::stringstream buffer;
                            buffer << doc_file.rdbuf();
                            std::string content = buffer.str();

                            if (!content.empty())
                            {
                                engine_->IndexDocument(static_cast<size_t>(loaded_count), content);
                                loaded_count++;
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "[-] Error al procesar documento: " << e.what() << std::endl;
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[-] Error al cargar documentos existentes: " << e.what() << std::endl;
        }
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
        if (!request.IsValid())
        {
            return false;
        }

        engine_->IndexDocument(request.document_id, request.content);
        return true;
    }

    size_t SearchService::IndexDocuments(const Models::IndexDocumentsRequest& request)
    {
        size_t indexed_count = 0;
        try
        {
            // Usar el número óptimo de hilos basado en el hardware
            size_t num_threads = std::thread::hardware_concurrency();

            // Para conjuntos pequeños, usar menos hilos
            if (request.documents.size() < 1000)
            {
                num_threads = std::max(size_t(1), num_threads / 2);
            }

            // Usar un tamaño de batch apropiado
            size_t batch_size = std::max(size_t(100), request.documents.size() / (num_threads * 2));

            indexed_count = engine_->IndexDocuments(request.documents, num_threads, batch_size);
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
        stats.engine_type = "BM25 Concurrent";
        stats.version = "2.0.0";
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