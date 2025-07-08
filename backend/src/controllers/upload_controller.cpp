#include "controllers/upload_controller.hpp"
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include "crow/json.h"
#include "crow/multipart.h"
#include "models/search_models.hpp"

namespace
{
    // Obtener la ruta de datos de aplicación específica del sistema operativo
    std::filesystem::path get_app_data_dir()
    {
        std::filesystem::path data_dir;

#ifdef _WIN32
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

        // Crear el directorio si no existe
        std::error_code ec;
        std::filesystem::create_directories(data_dir, ec);

        if (ec)
        {
            std::cerr << "[-] Error al crear directorio de datos: " << ec.message() << std::endl;
            data_dir = std::filesystem::current_path() / "data";
            std::filesystem::create_directories(data_dir);
        }

        std::cout << "[+] Directorio de datos: " << data_dir << std::endl;
        return data_dir;
    }

    // --- Rutas absolutas basadas en el directorio de datos del sistema ---
    const std::filesystem::path DATA_ROOT = get_app_data_dir();
    const std::filesystem::path DOCS_PATH = DATA_ROOT / "docs";
    const std::filesystem::path LAST_ID_FILE = DATA_ROOT / "last_id.txt";
    const std::filesystem::path DOC_INDEX_FILE = DATA_ROOT / "document_index.json";

    int get_next_id()
    {
        // Asegurar que el directorio existe
        std::error_code ec;
        std::filesystem::create_directories(DATA_ROOT, ec);
        if (ec)
        {
            std::cerr << "[-] Error al crear directorio de datos: " << ec.message() << std::endl;
            return 1;
        }

        int last_id = 0;

        // Leer último ID si el archivo existe
        if (std::filesystem::exists(LAST_ID_FILE))
        {
            std::ifstream in(LAST_ID_FILE);
            if (in.is_open())
            {
                in >> last_id;
                if (in.fail())
                {
                    std::cerr << "[-] Error al leer last_id.txt, reiniciando desde 0" << std::endl;
                    last_id = 0;
                }
            }
            else
            {
                std::cerr << "[-] No se pudo abrir last_id.txt para lectura" << std::endl;
            }
        }

        int next_id = last_id + 1;

        // Guardar nuevo ID
        std::ofstream out(LAST_ID_FILE);
        if (out.is_open())
        {
            out << next_id;
            if (out.fail())
            {
                std::cerr << "[-] Error al escribir last_id.txt" << std::endl;
            }
        }
        else
        {
            std::cerr << "[-] No se pudo abrir last_id.txt para escritura" << std::endl;
        }

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

        // Leer índice existente si existe
        if (std::filesystem::exists(DOC_INDEX_FILE))
        {
            std::ifstream in(DOC_INDEX_FILE);
            if (in.is_open())
            {
                try
                {
                    in >> index;
                    if (!index.is_array())
                    {
                        std::cerr
                            << "[-] document_index.json no contiene un array válido, reiniciando"
                            << std::endl;
                        index = nlohmann::json::array();
                    }
                }
                catch (const nlohmann::json::parse_error& e)
                {
                    std::cerr << "[-] Error al parsear document_index.json: " << e.what()
                              << std::endl;
                    std::cerr << "[-] Reiniciando índice desde cero" << std::endl;
                    index = nlohmann::json::array();
                }
            }
            else
            {
                std::cerr << "[-] No se pudo abrir document_index.json para lectura" << std::endl;
                index = nlohmann::json::array();
            }
        }
        else
        {
            // Archivo no existe, crear índice nuevo
            index = nlohmann::json::array();
            std::cout << "[+] Creando nuevo índice de documentos" << std::endl;
        }

        // Crear nueva entrada
        nlohmann::json new_entry;
        new_entry["id"] = doc_id;
        new_entry["filename"] = original_name;
        new_entry["path"] = new_path;
        new_entry["timestamp"] = std::time(nullptr);

        index.push_back(new_entry);

        // Guardar índice actualizado
        std::ofstream out(DOC_INDEX_FILE);
        if (out.is_open())
        {
            out << index.dump(4);
            if (out.fail())
            {
                std::cerr << "[-] Error al escribir document_index.json" << std::endl;
            }
        }
        else
        {
            std::cerr << "[-] No se pudo abrir document_index.json para escritura" << std::endl;
        }
    }

    // Extrae texto de un archivo (solo TXT por ahora)
    std::string extract_text(const std::string& filepath)
    {
        // Verificar que el archivo existe
        std::error_code ec;
        if (!std::filesystem::exists(filepath, ec))
        {
            std::cerr << "[-] Archivo no existe: " << filepath << std::endl;
            return "";
        }

        if (ec)
        {
            std::cerr << "[-] Error al verificar archivo: " << ec.message() << std::endl;
            return "";
        }

        // Verificar que es un archivo regular
        if (!std::filesystem::is_regular_file(filepath, ec))
        {
            std::cerr << "[-] No es un archivo regular: " << filepath << std::endl;
            return "";
        }

        // Leer contenido del archivo
        std::ifstream file(filepath, std::ios::in);
        if (!file.is_open())
        {
            std::cerr << "[-] No se pudo abrir archivo para lectura: " << filepath << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        if (file.bad())
        {
            std::cerr << "[-] Error al leer archivo: " << filepath << std::endl;
            return "";
        }

        std::string content = buffer.str();

        return content;
    }

} // namespace

namespace DocuTrace::Controllers
{
    UploadController::UploadController(std::shared_ptr<Services::SearchService> search_service)
        : search_service_(std::move(search_service))
    {
    }

    void UploadController::RegisterRoutes(crow::App<crow::CORSHandler>& app)
    {
        CROW_ROUTE(app, "/api/upload")
            .methods("POST"_method)(
                [this](const crow::request& req)
                {
                    crow::multipart::message msg(req);
                    auto file_part = msg.get_part_by_name("file");

                    // Verificar si encontramos la parte del archivo
                    if (file_part.headers.empty())
                    {
                        return crow::response(400, "{\"error\": \"No se encontró la parte del "
                                                   "archivo en el formulario.\"}");
                    }

                    std::string original_filename;

                    // Primero intentar obtener el nombre desde Content-Disposition
                    auto header_it = file_part.headers.find("Content-Disposition");
                    if (header_it != file_part.headers.end())
                    {
                        const auto& disp_header = header_it->second.value;
                        size_t filename_pos = disp_header.find("filename=");
                        if (filename_pos != std::string::npos)
                        {
                            std::string temp_filename =
                                disp_header.substr(filename_pos + strlen("filename="));

                            // Limpiar espacios y comillas
                            size_t first = temp_filename.find_first_not_of(" \"");
                            size_t last = temp_filename.find_last_not_of(" \"");

                            if (first != std::string::npos && last != std::string::npos)
                            {
                                original_filename = temp_filename.substr(first, (last - first + 1));
                            }
                        }
                    }

                    // Si no pudimos obtener el nombre del archivo, usar un nombre por defecto
                    if (original_filename.empty())
                    {
                        original_filename = "uploaded_file.txt";
                        std::cout << "[!] Usando nombre de archivo por defecto: "
                                  << original_filename << std::endl;
                    }

                    std::string extension = get_file_extension(original_filename);
                    if (extension != ".txt")
                    {
                        return crow::response(
                            400, "{\"error\": \"Por ahora, solo se permiten archivos .txt\"}");
                    }

                    // --- Lógica de persistencia e indexación ---
                    int new_id = get_next_id();
                    std::filesystem::path target_dir = DOCS_PATH / "txt";
                    std::filesystem::create_directories(target_dir);

                    const std::string new_internal_filename = std::to_string(new_id) + extension;
                    const std::string file_path_str = (target_dir / new_internal_filename).string();

                    // Guardar archivo
                    std::ofstream out_file(file_path_str, std::ios::binary);
                    if (!out_file)
                    {
                        return crow::response(
                            500, "{\"error\": \"No se pudo guardar el archivo en el servidor.\"}");
                    }
                    out_file.write(file_part.body.data(), file_part.body.length());
                    out_file.close();

                    // Actualizar índice
                    update_document_index(new_id, original_filename, file_path_str);

                    // Extraer texto e indexar
                    std::string content = extract_text(file_path_str);
                    if (!content.empty())
                    {
                        Models::IndexDocumentRequest index_req{static_cast<size_t>(new_id),
                                                               content};
                        search_service_->IndexDocument(index_req);
                    }

                    crow::json::wvalue response;
                    response["message"] =
                        "Archivo '" + original_filename + "' subido e indexado exitosamente.";
                    response["doc_id"] = new_id;
                    response["path"] = file_path_str;
                    return crow::response(201, response);
                });
    }

} // namespace DocuTrace::Controllers