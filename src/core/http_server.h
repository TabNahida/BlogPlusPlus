#pragma once

#include "core/http_types.h"

#include <filesystem>
#include <functional>
#include <unordered_map>

namespace blogpp {

class StaticHttpServer {
public:
    using DynamicHandler = std::function<bool(const HttpRequest&, HttpResponse&)>;

    explicit StaticHttpServer(std::filesystem::path root_dir, size_t file_cache_limit_bytes = 16 * 1024 * 1024);
    bool start(int port, DynamicHandler handler = {});

private:
    struct CachedFile {
        std::string content;
        std::string mime_type;
        std::filesystem::file_time_type last_write_time{};
        size_t size = 0;
    };

    std::filesystem::path root_dir_;
    size_t file_cache_limit_bytes_ = 0;
    size_t file_cache_size_ = 0;
    std::unordered_map<std::string, CachedFile> file_cache_;

    std::filesystem::path resolve_path(const std::string& request_path) const;
    bool read_static_response(const std::filesystem::path& file, HttpResponse& response);
};

}  // namespace blogpp
