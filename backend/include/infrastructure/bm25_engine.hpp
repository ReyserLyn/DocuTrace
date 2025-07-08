#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

namespace DocuTrace::Infrastructure
{
    // Forward declaration para evitar dependencias circulares
    namespace Models
    {
        struct SearchResult;
    }

    /**
     * @brief Resultado de una búsqueda con puntuación BM25
     */
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
     * @brief Índice invertido optimizado para BM25
     * @note Implementación de infraestructura - maneja almacenamiento
     */
    class InvertedIndex
    {
      private:
        // Documentos que contienen cada palabra
        std::map<std::string, std::set<int>> document_frequency_;
        // Frecuencia de cada palabra en cada documento
        std::map<std::string, std::map<int, int>> term_frequency_;

      public:
        void AddTerm(const std::string& term, int document_id);
        int GetDocumentFrequency(const std::string& term, int document_id) const;
        int GetIndexFrequency(const std::string& term) const;
        std::set<int> GetDocuments(const std::string& term) const;
        void Clear();
    };

    /**
     * @brief Tabla de longitudes de documentos
     * @note Optimizada con cache para longitud promedio
     */
    class DocumentLengthTable
    {
      private:
        std::map<int, int> lengths_;
        mutable double average_length_cache_ = -1.0;
        mutable bool cache_valid_ = false;

      public:
        void AddDocument(int document_id, int length);
        int GetLength(int document_id) const;
        double GetAverageLength() const;
        void Clear();
    };

    /**
     * @brief Motor de búsqueda BM25 completo
     * @note Capa de infraestructura - implementación concreta del algoritmo
     */
    class BM25Engine
    {
      private:
        static constexpr double K1 = 1.2;
        static constexpr double B = 0.75;

        InvertedIndex index_;
        DocumentLengthTable document_lengths_;
        std::vector<std::string> documents_;

        double CalculateBM25Score(double n, double f, double N, double dl, double avdl) const;
        std::vector<std::string> TokenizeAndNormalize(const std::string& text) const;

      public:
        BM25Engine() = default;
        ~BM25Engine() = default;

        // No copyable pero movible
        BM25Engine(const BM25Engine&) = delete;
        BM25Engine& operator=(const BM25Engine&) = delete;
        BM25Engine(BM25Engine&&) = default;
        BM25Engine& operator=(BM25Engine&&) = default;

        void IndexDocument(const std::string& content);
        void IndexDocuments(const std::vector<std::string>& documents);
        std::vector<SearchResult> Search(const std::string& query, size_t max_results = 50) const;
        void Clear();
        size_t GetDocumentCount() const
        {
            return documents_.size();
        }
    };

} // namespace DocuTrace::Infrastructure