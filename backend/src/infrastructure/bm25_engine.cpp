#include "infrastructure/bm25_engine.hpp"
#include <algorithm>
#include <cmath>
#include "shared/text_utils.hpp"

namespace DocuTrace::Infrastructure
{

    // ============================================================================
    // IMPLEMENTACIÓN DE InvertedIndex
    // ============================================================================

    void InvertedIndex::AddTerm(const std::string& term, int document_id)
    {
        document_frequency_[term].insert(document_id);
        term_frequency_[term][document_id]++;
    }

    int InvertedIndex::GetDocumentFrequency(const std::string& term, int document_id) const
    {
        auto term_it = term_frequency_.find(term);
        if (term_it == term_frequency_.end())
        {
            return 0;
        }

        auto doc_it = term_it->second.find(document_id);
        return (doc_it != term_it->second.end()) ? doc_it->second : 0;
    }

    int InvertedIndex::GetIndexFrequency(const std::string& term) const
    {
        auto it = document_frequency_.find(term);
        return (it != document_frequency_.end()) ? static_cast<int>(it->second.size()) : 0;
    }

    std::set<int> InvertedIndex::GetDocuments(const std::string& term) const
    {
        auto it = document_frequency_.find(term);
        return (it != document_frequency_.end()) ? it->second : std::set<int>{};
    }

    void InvertedIndex::Clear()
    {
        document_frequency_.clear();
        term_frequency_.clear();
    }

    // ============================================================================
    // IMPLEMENTACIÓN DE DocumentLengthTable
    // ============================================================================

    void DocumentLengthTable::AddDocument(int document_id, int length)
    {
        lengths_[document_id] = length;
        cache_valid_ = false; // Invalidar cache
    }

    int DocumentLengthTable::GetLength(int document_id) const
    {
        auto it = lengths_.find(document_id);
        return (it != lengths_.end()) ? it->second : 0;
    }

    double DocumentLengthTable::GetAverageLength() const
    {
        if (cache_valid_)
        {
            return average_length_cache_;
        }

        if (lengths_.empty())
        {
            return 0.0;
        }

        double sum = 0.0;
        for (const auto& [doc_id, length] : lengths_)
        {
            sum += length;
        }

        average_length_cache_ = sum / static_cast<double>(lengths_.size());
        cache_valid_ = true;
        return average_length_cache_;
    }

    void DocumentLengthTable::Clear()
    {
        lengths_.clear();
        cache_valid_ = false;
        average_length_cache_ = -1.0;
    }

    // ============================================================================
    // IMPLEMENTACIÓN DE BM25Engine
    // ============================================================================

    double BM25Engine::CalculateBM25Score(double n, double f, double N, double dl,
                                          double avdl) const
    {
        // Implementación del algoritmo BM25
        double idf = std::log((N - n + 0.5) / (n + 0.5));
        double tf_component = (f * (K1 + 1)) / (f + K1 * (1 - B + B * dl / avdl));
        return idf * tf_component;
    }

    std::vector<std::string> BM25Engine::TokenizeAndNormalize(const std::string& text) const
    {
        return Shared::TextUtils::normalizeForSearch(text);
    }

    void BM25Engine::IndexDocument(const std::string& content)
    {
        int document_id = static_cast<int>(documents_.size());
        documents_.push_back(content);

        std::vector<std::string> tokens = TokenizeAndNormalize(content);
        document_lengths_.AddDocument(document_id, static_cast<int>(tokens.size()));

        for (const std::string& token : tokens)
        {
            index_.AddTerm(token, document_id);
        }
    }

    void BM25Engine::IndexDocuments(const std::vector<std::string>& documents)
    {
        for (const std::string& doc : documents)
        {
            IndexDocument(doc);
        }
    }

    std::vector<SearchResult> BM25Engine::Search(const std::string& query, size_t max_results) const
    {
        if (documents_.empty())
        {
            return {};
        }

        std::vector<std::string> query_tokens = TokenizeAndNormalize(query);
        if (query_tokens.empty())
        {
            return {};
        }

        std::map<int, double> document_scores;
        const double total_documents = static_cast<double>(documents_.size());
        const double avg_doc_length = document_lengths_.GetAverageLength();

        // Calcular score para cada término de la query
        for (const std::string& token : query_tokens)
        {
            std::set<int> matching_docs = index_.GetDocuments(token);
            const int docs_with_term = index_.GetIndexFrequency(token);

            for (int doc_id : matching_docs)
            {
                const int term_freq = index_.GetDocumentFrequency(token, doc_id);
                const int doc_length = document_lengths_.GetLength(doc_id);

                double score = CalculateBM25Score(static_cast<double>(docs_with_term),
                                                  static_cast<double>(term_freq), total_documents,
                                                  static_cast<double>(doc_length), avg_doc_length);

                document_scores[doc_id] += score;
            }
        }

        // Convertir a vector y ordenar por score
        std::vector<std::pair<double, int>> score_pairs;
        for (const auto& [doc_id, score] : document_scores)
        {
            if (score > 0.0)
            {
                score_pairs.emplace_back(score, doc_id);
            }
        }

        // Ordenar por score descendente
        std::sort(score_pairs.begin(), score_pairs.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });

        // Crear resultados finales
        std::vector<SearchResult> results;
        results.reserve(std::min(max_results, score_pairs.size()));

        for (size_t i = 0; i < std::min(max_results, score_pairs.size()); ++i)
        {
            const auto& [score, doc_id] = score_pairs[i];
            results.emplace_back(documents_[doc_id], score, doc_id);
        }

        return results;
    }

    void BM25Engine::Clear()
    {
        index_.Clear();
        document_lengths_.Clear();
        documents_.clear();
    }

} // namespace DocuTrace::Infrastructure