#include "infrastructure/bm25_engine.hpp"
#include <algorithm>
#include <cmath>
#include <future>
#include <numeric>
#include <thread>
#include "shared/text_utils.hpp"

namespace DocuTrace::Infrastructure
{
    // ============================================================================
    // IMPLEMENTACIÓN DE InvertedIndex
    // ============================================================================

    void InvertedIndex::AddTerm(const std::string& term, int document_id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        document_frequency_[term].insert(document_id);
        term_frequency_[term][document_id]++;
    }

    void InvertedIndex::AddTerms(const std::vector<std::string>& terms, int document_id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& term : terms)
        {
            document_frequency_[term].insert(document_id);
            term_frequency_[term][document_id]++;
        }
    }

    int InvertedIndex::GetDocumentFrequency(const std::string& term, int document_id) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
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
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = document_frequency_.find(term);
        return (it != document_frequency_.end()) ? static_cast<int>(it->second.size()) : 0;
    }

    std::set<int> InvertedIndex::GetDocuments(const std::string& term) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = document_frequency_.find(term);
        return (it != document_frequency_.end()) ? it->second : std::set<int>{};
    }

    void InvertedIndex::Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        document_frequency_.clear();
        term_frequency_.clear();
    }

    // ============================================================================
    // IMPLEMENTACIÓN DE DocumentLengthTable
    // ============================================================================

    void DocumentLengthTable::AddDocument(int document_id, int length)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        lengths_[document_id] = length;
        cache_valid_ = false;
    }

    int DocumentLengthTable::GetLength(int document_id) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = lengths_.find(document_id);
        return (it != lengths_.end()) ? it->second : 0;
    }

    double DocumentLengthTable::GetAverageLength() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cache_valid_)
        {
            return average_length_cache_;
        }

        if (lengths_.empty())
        {
            return 0.0;
        }

        double sum =
            std::accumulate(lengths_.begin(), lengths_.end(), 0.0,
                            [](double acc, const auto& pair) { return acc + pair.second; });

        average_length_cache_ = sum / static_cast<double>(lengths_.size());
        cache_valid_ = true;
        return average_length_cache_;
    }

    void DocumentLengthTable::Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
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
        double idf = std::log((N - n + 0.5) / (n + 0.5));
        double tf_component = (f * (K1 + 1)) / (f + K1 * (1 - B + B * dl / avdl));
        return idf * tf_component;
    }

    std::vector<std::string> BM25Engine::TokenizeAndNormalize(const std::string& text) const
    {
        return Shared::TextUtils::normalizeForSearch(text);
    }

    size_t BM25Engine::GetOptimalThreadCount(size_t document_count) const
    {
        // Si hay pocos documentos, usar menos hilos
        const size_t hardware_threads = std::thread::hardware_concurrency();
        const size_t min_docs_per_thread = 100;

        size_t optimal_threads =
            std::min(hardware_threads, std::max(size_t(1), document_count / min_docs_per_thread));

        return optimal_threads;
    }

    void BM25Engine::IndexDocument(const std::string& content)
    {
        std::vector<std::string> tokens = TokenizeAndNormalize(content);

        std::lock_guard<std::mutex> lock(documents_mutex_);
        int document_id = static_cast<int>(documents_.size());
        documents_.push_back(content);
        document_lengths_.AddDocument(document_id, static_cast<int>(tokens.size()));
        index_.AddTerms(tokens, document_id);
    }

    void BM25Engine::IndexDocumentBatch(const std::vector<std::string>& batch, size_t start_id)
    {
        for (size_t i = 0; i < batch.size(); ++i)
        {
            const auto& content = batch[i];
            std::vector<std::string> tokens = TokenizeAndNormalize(content);

            size_t doc_id = start_id + i;
            document_lengths_.AddDocument(static_cast<int>(doc_id),
                                          static_cast<int>(tokens.size()));
            index_.AddTerms(tokens, static_cast<int>(doc_id));
        }
    }

    size_t BM25Engine::IndexDocuments(const std::vector<std::string>& documents, size_t num_threads,
                                      size_t batch_size)
    {
        if (documents.empty())
        {
            return 0;
        }

        // Determinar número óptimo de hilos si no se especificó
        if (num_threads == 0)
        {
            num_threads = GetOptimalThreadCount(documents.size());
        }

        // Reservar espacio para los documentos
        {
            std::lock_guard<std::mutex> lock(documents_mutex_);
            documents_.reserve(documents_.size() + documents.size());
        }

        // Dividir documentos en batches
        std::vector<std::future<void>> futures;
        std::vector<std::vector<std::string>> batches;

        size_t current_pos = 0;
        while (current_pos < documents.size())
        {
            size_t remaining = documents.size() - current_pos;
            size_t current_batch_size = std::min(batch_size, remaining);

            std::vector<std::string> batch(documents.begin() + current_pos,
                                           documents.begin() + current_pos + current_batch_size);

            size_t start_id;
            {
                std::lock_guard<std::mutex> lock(documents_mutex_);
                start_id = documents_.size();
                documents_.insert(documents_.end(), batch.begin(), batch.end());
            }

            futures.push_back(std::async(std::launch::async, &BM25Engine::IndexDocumentBatch, this,
                                         std::move(batch), start_id));

            current_pos += current_batch_size;

            // Limitar el número de hilos activos
            if (futures.size() >= num_threads)
            {
                futures.front().wait();
                futures.erase(futures.begin());
            }
        }

        // Esperar que terminen todos los hilos
        for (auto& future : futures)
        {
            future.wait();
        }

        return documents.size();
    }

    std::vector<SearchResult> BM25Engine::Search(const std::string& query, size_t max_results) const
    {
        std::lock_guard<std::mutex> lock(documents_mutex_);

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
        score_pairs.reserve(document_scores.size());

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
        std::lock_guard<std::mutex> lock(documents_mutex_);
        index_.Clear();
        document_lengths_.Clear();
        documents_.clear();
    }

} // namespace DocuTrace::Infrastructure