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

    void BM25Engine::IndexDocument(size_t document_id, const std::string& content)
    {
        std::vector<std::string> tokens = TokenizeAndNormalize(content);

        std::lock_guard<std::mutex> lock(documents_mutex_);

        // Usar document_id (size_t) para el vector, eliminando la advertencia.
        if (documents_.size() <= document_id)
        {
            documents_.resize(document_id + 1);
        }
        documents_[document_id] = content;

        // Usar un ID de tipo int para las clases auxiliares que lo requieren.
        int doc_id_int = static_cast<int>(document_id);
        document_lengths_.AddDocument(doc_id_int, static_cast<int>(tokens.size()));
        index_.AddTerms(tokens, doc_id_int);
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

        std::vector<double> scores(documents_.size(), 0.0);
        double N = static_cast<double>(documents_.size());
        double avdl = document_lengths_.GetAverageLength();

        for (const auto& token : query_tokens)
        {
            double n = static_cast<double>(index_.GetIndexFrequency(token));
            if (n == 0)
            {
                continue;
            }

            auto docs_with_token = index_.GetDocuments(token);

            for (int doc_id : docs_with_token)
            {
                double f = static_cast<double>(index_.GetDocumentFrequency(token, doc_id));
                double dl = static_cast<double>(document_lengths_.GetLength(doc_id));
                double score = CalculateBM25Score(n, f, N, dl, avdl);
                scores[doc_id] += score;
            }
        }

        // Crear resultados y ordenarlos
        std::vector<SearchResult> results;
        for (size_t i = 0; i < scores.size(); ++i)
        {
            if (scores[i] != 0.0)
            {
                if (i < documents_.size() && !documents_[i].empty())
                {
                    results.emplace_back(documents_[i], scores[i], static_cast<int>(i));
                }
            }
        }

        std::sort(results.begin(), results.end(),
                  [](const SearchResult& a, const SearchResult& b) { return a.score > b.score; });

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