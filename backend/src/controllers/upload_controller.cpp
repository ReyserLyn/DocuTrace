#include "controllers/upload_controller.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include "crow/json.h"
#include "models/search_models.hpp"

namespace
{
    // --- Rutas robustas relativas al directorio de compilación ---
    const std::filesystem::path DATA_ROOT = "data";
    const std::filesystem::path DOCS_PATH = DATA_ROOT / "docs";
    const std::filesystem::path LAST_ID_FILE = DATA_ROOT / "last_id.txt";
    const std::filesystem::path DOC_INDEX_FILE = DATA_ROOT / "document_index.json";

    int get_next_id()
    {
        std::filesystem::create_directories(DATA_ROOT);
        int last_id = 0;
        std::ifstream in(LAST_ID_FILE);
        if (in)
        {
            in >> last_id;
            in.close();
        }

        int next_id = last_id + 1;

        std::ofstream out(LAST_ID_FILE);
        out << next_id;
        out.close();

        return next_id;
    }

    // Extrae la extensión de un nombre de archivo
    std::string get_file_extension(const std::string& filename)
    {
        return std::filesystem::path(filename).extension().string();
    }

    // Actualiza el índice de documentos con la información del nuevo archivo.
    void update_document_index(int doc_id, const std::string& original_name,
                               const std::string& new_path)
    {
        nlohmann::json index;

        std::ifstream in(DOC_INDEX_FILE);
        if (in)
        {
            try
            {
                in >> index;
            }
            catch (const nlohmann::json::parse_error& e)
            {
                std::cerr << "Advertencia: no se pudo parsear document_index.json: " << e.what()
                          << std::endl;
                index = nlohmann::json::array();
            }
            in.close();
        }
        else
        {
            index = nlohmann::json::array();
        }

        if (!index.is_array())
        {
            index = nlohmann::json::array();
        }

        nlohmann::json new_entry;
        new_entry["id"] = doc_id;
        new_entry["filename"] = original_name;
        new_entry["path"] = new_path;

        index.push_back(new_entry);

        std::ofstream out(DOC_INDEX_FILE);
        out << index.dump(4);
        out.close();
    }

    // Extrae texto de un archivo (solo TXT por ahora)
    std::string extract_text(const std::string& filepath)
    {
        // Asegurarse de que el archivo existe antes de leerlo
        if (!std::filesystem::exists(filepath))
            return "";

        std::ifstream file(filepath);
        if (!file)
            return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

} // namespace

namespace DocuTrace::Controllers
{
    UploadController::UploadController(std::shared_ptr<Services::SearchService> search_service)
        : search_service_(std::move(search_service))
    {
    }

    void UploadController::RegisterRoutes(crow::SimpleApp& app)
    {
        CROW_ROUTE(app, "/api/upload/<string>")
            .methods("POST"_method)(
                [this](const crow::request& req, const std::string& filename)
                {
                    if (filename.empty())
                    {
                        return crow::response(
                            400, "{\"error\": \"El nombre de archivo no puede estar vacío.\"}");
                    }

                    std::string extension = get_file_extension(filename);
                    if (extension != ".txt")
                    {
                        return crow::response(
                            400, "{\"error\": \"Por ahora, solo se permiten archivos .txt\"}");
                    }

                    // --- Enfoque robusto para encontrar la parte del archivo ---
                    if (req.body.empty())
                    {
                        return crow::response(400,
                                              "{\"error\": \"El cuerpo del request está vacío.\"}");
                    }

                    // --- Lógica de persistencia e indexación ---

                    // 1. Obtener nuevo ID y preparar rutas
                    int new_id = get_next_id();
                    std::filesystem::path target_dir = DOCS_PATH / "txt";
                    std::filesystem::create_directories(target_dir);

                    const std::string new_internal_filename = std::to_string(new_id) + extension;
                    const std::string file_path_str = (target_dir / new_internal_filename).string();

                    // 2. Guardar archivo con el nuevo ID
                    std::ofstream out_file(file_path_str, std::ios::binary);
                    if (!out_file)
                    {
                        return crow::response(
                            500, "{\"error\": \"No se pudo guardar el archivo en el servidor.\"}");
                    }
                    out_file.write(req.body.data(), req.body.length());
                    out_file.close();

                    // 3. Actualizar el índice de documentos
                    update_document_index(new_id, filename, file_path_str);

                    // 4. Extraer texto
                    std::string content = extract_text(file_path_str);

                    // 5. Indexar el documento en el servicio
                    if (!content.empty())
                    {
                        Models::IndexDocumentRequest index_req{static_cast<size_t>(new_id),
                                                               content};
                        search_service_->IndexDocument(index_req);
                    }

                    crow::json::wvalue response;
                    response["message"] =
                        "Archivo '" + filename + "' subido e indexado exitosamente.";
                    response["doc_id"] = new_id;
                    response["path"] = file_path_str;
                    return crow::response(201, response);
                });
    }

} // namespace DocuTrace::Controllers