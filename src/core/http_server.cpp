#include "core/http_server.h"

#include "core/utils.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace blogpp {

namespace {

#ifdef _WIN32
using socket_t = SOCKET;
const socket_t kInvalidSocket = INVALID_SOCKET;
#else
using socket_t = int;
const socket_t kInvalidSocket = -1;
#endif

void close_socket(socket_t socket_fd) {
    if (socket_fd == kInvalidSocket) {
        return;
    }
#ifdef _WIN32
    closesocket(socket_fd);
#else
    close(socket_fd);
#endif
}

bool send_all(socket_t socket_fd, const std::string& payload) {
    size_t sent_total = 0;
    while (sent_total < payload.size()) {
        const int sent = send(socket_fd,
                              payload.data() + sent_total,
                              static_cast<int>(payload.size() - sent_total),
                              0);
        if (sent <= 0) {
            return false;
        }
        sent_total += static_cast<size_t>(sent);
    }
    return true;
}

std::string reason_phrase(int status_code) {
    switch (status_code) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 413:
            return "Payload Too Large";
        case 500:
            return "Internal Server Error";
        default:
            return "Unknown";
    }
}

std::string build_response(const HttpResponse& response) {
    const std::string status_text =
        response.status_text.empty() ? reason_phrase(response.status_code) : response.status_text;

    std::ostringstream stream;
    stream << "HTTP/1.1 " << response.status_code << " " << status_text << "\r\n"
           << "Content-Type: " << response.content_type << "\r\n"
           << "Content-Length: " << response.body.size() << "\r\n"
           << "Connection: close\r\n"
           << "Cache-Control: no-store\r\n\r\n"
           << response.body;
    return stream.str();
}

std::unordered_map<std::string, std::string> parse_query(const std::string& query) {
    std::unordered_map<std::string, std::string> parsed;
    if (query.empty()) {
        return parsed;
    }
    for (const auto& pair : split(query, '&')) {
        if (pair.empty()) {
            continue;
        }
        auto pos = pair.find('=');
        const std::string key = url_decode(pos == std::string::npos ? pair : pair.substr(0, pos));
        const std::string value = url_decode(pos == std::string::npos ? "" : pair.substr(pos + 1));
        if (!key.empty()) {
            parsed[key] = value;
        }
    }
    return parsed;
}

std::string read_request_payload(socket_t client_fd, size_t max_size = 2 * 1024 * 1024) {
    std::string payload;
    payload.reserve(8192);
    char chunk[8192] = {0};

    size_t header_end = std::string::npos;
    size_t expected_total = 0;
    while (payload.size() < max_size) {
        const int received = recv(client_fd, chunk, static_cast<int>(sizeof(chunk)), 0);
        if (received <= 0) {
            break;
        }
        payload.append(chunk, static_cast<size_t>(received));

        if (header_end == std::string::npos) {
            header_end = payload.find("\r\n\r\n");
            if (header_end != std::string::npos) {
                const auto headers_text = payload.substr(0, header_end + 4);
                size_t content_length = 0;

                std::istringstream lines(headers_text);
                std::string line;
                while (std::getline(lines, line)) {
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    const auto sep = line.find(':');
                    if (sep == std::string::npos) {
                        continue;
                    }
                    const auto key = to_lower(trim(line.substr(0, sep)));
                    const auto value = trim(line.substr(sep + 1));
                    if (key == "content-length") {
                        try {
                            content_length = static_cast<size_t>(std::stoull(value));
                        } catch (...) {
                            content_length = 0;
                        }
                        break;
                    }
                }
                expected_total = header_end + 4 + content_length;
            }
        }

        if (expected_total > 0 && payload.size() >= expected_total) {
            break;
        }
    }

    if (payload.size() > max_size) {
        return {};
    }
    return payload;
}

bool parse_http_request(const std::string& raw, HttpRequest& request) {
    const auto header_end = raw.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return false;
    }

    const auto header_block = raw.substr(0, header_end);
    request.body = raw.substr(header_end + 4);

    std::istringstream input(header_block);
    std::string request_line;
    if (!std::getline(input, request_line)) {
        return false;
    }
    if (!request_line.empty() && request_line.back() == '\r') {
        request_line.pop_back();
    }

    std::istringstream first_line(request_line);
    std::string target;
    std::string version;
    first_line >> request.method >> target >> version;
    if (request.method.empty() || target.empty() || version.empty()) {
        return false;
    }

    auto query_pos = target.find('?');
    if (query_pos == std::string::npos) {
        request.path = target;
    } else {
        request.path = target.substr(0, query_pos);
        request.query = target.substr(query_pos + 1);
    }
    if (request.path.empty()) {
        request.path = "/";
    }
    request.query_params = parse_query(request.query);

    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const auto sep = line.find(':');
        if (sep == std::string::npos) {
            continue;
        }
        const auto key = to_lower(trim(line.substr(0, sep)));
        const auto value = trim(line.substr(sep + 1));
        if (!key.empty()) {
            request.headers[key] = value;
        }
    }

    return true;
}

}  // namespace

StaticHttpServer::StaticHttpServer(fs::path root_dir, size_t file_cache_limit_bytes)
    : root_dir_(std::move(root_dir)),
      file_cache_limit_bytes_(file_cache_limit_bytes) {}

fs::path StaticHttpServer::resolve_path(const std::string& request_path) const {
    std::string path = request_path;
    const auto query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        path = path.substr(0, query_pos);
    }
    if (path.empty() || path == "/") {
        path = "/index.html";
    }
    if (path.find("..") != std::string::npos) {
        return {};
    }

    if (path.front() == '/') {
        path.erase(path.begin());
    }

    fs::path candidate = root_dir_ / path;
    if (!fs::exists(candidate) && !candidate.has_extension()) {
        candidate /= "index.html";
    } else if (fs::is_directory(candidate)) {
        candidate /= "index.html";
    }

    std::error_code ec;
    if (!fs::exists(candidate, ec) || !fs::is_regular_file(candidate, ec)) {
        return {};
    }

    const auto canonical_root = fs::weakly_canonical(root_dir_, ec);
    if (ec) {
        return {};
    }
    const auto canonical_candidate = fs::weakly_canonical(candidate, ec);
    if (ec) {
        return {};
    }
    const auto rel = canonical_candidate.lexically_relative(canonical_root);
    if (rel.empty() || rel.string().rfind("..", 0) == 0) {
        return {};
    }
    return canonical_candidate;
}

bool StaticHttpServer::read_static_response(const fs::path& file, HttpResponse& response) {
    std::error_code ec;
    const auto current_write_time = fs::last_write_time(file, ec);
    const auto key = file.string();

    if (!ec) {
        const auto cache_it = file_cache_.find(key);
        if (cache_it != file_cache_.end() && cache_it->second.last_write_time == current_write_time) {
            response.status_code = 200;
            response.status_text = "OK";
            response.content_type = cache_it->second.mime_type;
            response.body = cache_it->second.content;
            return true;
        }
    }

    std::ifstream input(file, std::ios::binary);
    if (!input) {
        return false;
    }
    std::ostringstream content_stream;
    content_stream << input.rdbuf();
    auto content = content_stream.str();
    const auto mime = guess_mime_type(file);

    response.status_code = 200;
    response.status_text = "OK";
    response.content_type = mime;
    response.body = content;

    if (file_cache_limit_bytes_ == 0 || content.size() > file_cache_limit_bytes_) {
        return true;
    }

    auto existing = file_cache_.find(key);
    if (existing != file_cache_.end()) {
        file_cache_size_ -= existing->second.size;
        file_cache_.erase(existing);
    }

    if (file_cache_size_ + content.size() > file_cache_limit_bytes_) {
        file_cache_.clear();
        file_cache_size_ = 0;
    }

    if (file_cache_size_ + content.size() <= file_cache_limit_bytes_) {
        file_cache_[key] = CachedFile{
            content,
            mime,
            current_write_time,
            content.size(),
        };
        file_cache_size_ += content.size();
    }

    return true;
}

bool StaticHttpServer::start(int port, DynamicHandler handler) {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }
#endif

    socket_t server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == kInvalidSocket) {
        std::cerr << "Failed to create socket\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Failed to bind on port " << port << "\n";
        close_socket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    if (listen(server_fd, 32) < 0) {
        std::cerr << "Failed to listen on port " << port << "\n";
        close_socket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    std::cout << "Serving " << root_dir_.string() << " at http://localhost:" << port << std::endl;

    while (true) {
        sockaddr_in client_addr{};
#ifdef _WIN32
        int client_len = sizeof(client_addr);
#else
        socklen_t client_len = sizeof(client_addr);
#endif
        socket_t client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client_fd == kInvalidSocket) {
            continue;
        }

        HttpResponse response;
        const auto raw_request = read_request_payload(client_fd);
        if (raw_request.empty()) {
            response.status_code = 400;
            response.status_text = "Bad Request";
            response.content_type = "text/plain; charset=utf-8";
            response.body = "Invalid HTTP request.";
            send_all(client_fd, build_response(response));
            close_socket(client_fd);
            continue;
        }

        HttpRequest request;
        if (!parse_http_request(raw_request, request)) {
            response.status_code = 400;
            response.status_text = "Bad Request";
            response.content_type = "text/plain; charset=utf-8";
            response.body = "Malformed request line.";
            send_all(client_fd, build_response(response));
            close_socket(client_fd);
            continue;
        }

        bool handled = false;
        if (handler) {
            handled = handler(request, response);
        }

        if (!handled) {
            const auto method = to_lower(request.method);
            if (method != "get" && method != "head") {
                response.status_code = 405;
                response.status_text = "Method Not Allowed";
                response.content_type = "text/plain; charset=utf-8";
                response.body = "Only GET/HEAD are supported for static files.";
            } else {
                const auto file = resolve_path(request.path);
                if (file.empty()) {
                    response.status_code = 404;
                    response.status_text = "Not Found";
                    response.content_type = "text/html; charset=utf-8";
                    response.body = "<h1>404</h1><p>Page not found.</p>";
                } else if (!read_static_response(file, response)) {
                    response.status_code = 500;
                    response.status_text = "Internal Server Error";
                    response.content_type = "text/plain; charset=utf-8";
                    response.body = "Failed to read file.";
                }
            }
        }

        if (to_lower(request.method) == "head") {
            response.body.clear();
        }

        send_all(client_fd, build_response(response));
        close_socket(client_fd);
    }

    close_socket(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return true;
}

}  // namespace blogpp
