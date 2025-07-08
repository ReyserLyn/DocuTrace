export interface SearchRequest {
  query: string;
  limit?: number;
}

export interface SearchResult {
  document_id: number;
  content_preview: string;
  score: number;
}

export interface SearchResponse {
  total_results: number;
  results: SearchResult[];
  success: boolean;
}

export interface UploadResponse {
  message: string;
  doc_id: number;
  path: string;
}

export interface UploadError {
  type: "validation" | "network" | "server" | "unknown";
  message: string;
  details?: string;
}
