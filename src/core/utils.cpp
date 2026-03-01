#include "core/utils.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace blogpp {

std::string ltrim(const std::string& input) {
    const auto pos = input.find_first_not_of(" \t\r\n");
    return (pos == std::string::npos) ? "" : input.substr(pos);
}

std::string rtrim(const std::string& input) {
    const auto pos = input.find_last_not_of(" \t\r\n");
    return (pos == std::string::npos) ? "" : input.substr(0, pos + 1);
}

std::string trim(const std::string& input) {
    return rtrim(ltrim(input));
}

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::vector<std::string> split(const std::string& input, char delim) {
    std::vector<std::string> parts;
    std::string current;
    std::stringstream ss(input);
    while (std::getline(ss, current, delim)) {
        parts.push_back(current);
    }
    return parts;
}

std::vector<std::string> parse_csv(const std::string& input) {
    std::string value = trim(input);
    if (value.empty()) {
        return {};
    }
    if (value.front() == '[' && value.back() == ']') {
        value = value.substr(1, value.size() - 2);
    }
    std::vector<std::string> items;
    for (const auto& part : split(value, ',')) {
        const auto item = trim(part);
        if (!item.empty()) {
            items.push_back(item);
        }
    }
    return items;
}

std::string join(const std::vector<std::string>& values, const std::string& delim) {
    if (values.empty()) {
        return "";
    }
    std::ostringstream oss;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            oss << delim;
        }
        oss << values[i];
    }
    return oss.str();
}

std::string replace_all(std::string text, const std::string& from, const std::string& to) {
    if (from.empty()) {
        return text;
    }
    size_t start_pos = 0;
    while ((start_pos = text.find(from, start_pos)) != std::string::npos) {
        text.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return text;
}

std::string slugify(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    bool previous_dash = false;
    for (unsigned char ch : input) {
        if (std::isalnum(ch)) {
            out.push_back(static_cast<char>(std::tolower(ch)));
            previous_dash = false;
            continue;
        }
        if (ch == '+') {
            if (!out.empty() && out.back() != '-') {
                out.push_back('-');
            }
            out += "plus";
            out.push_back('-');
            previous_dash = true;
            continue;
        }
        if (ch == ' ' || ch == '-' || ch == '_' || ch == '.') {
            if (!previous_dash && !out.empty()) {
                out.push_back('-');
                previous_dash = true;
            }
        }
    }
    out = replace_all(out, "--", "-");
    while (!out.empty() && out.back() == '-') {
        out.pop_back();
    }
    if (out.empty()) {
        return "post";
    }
    return out;
}

std::string html_escape(const std::string& input) {
    std::string out = input;
    out = replace_all(out, "&", "&amp;");
    out = replace_all(out, "<", "&lt;");
    out = replace_all(out, ">", "&gt;");
    out = replace_all(out, "\"", "&quot;");
    out = replace_all(out, "'", "&#39;");
    return out;
}

std::string escape_json(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '\\':
                out += "\\\\";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                out += c;
                break;
        }
    }
    return out;
}

std::string read_text_file(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) {
        return "";
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

bool write_text_file(const std::filesystem::path& file, const std::string& content) {
    std::error_code ec;
    std::filesystem::create_directories(file.parent_path(), ec);
    std::ofstream out(file, std::ios::binary);
    if (!out) {
        return false;
    }
    out << content;
    return true;
}

bool file_exists(const std::filesystem::path& file) {
    std::error_code ec;
    return std::filesystem::exists(file, ec);
}

std::string current_date() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string current_datetime() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

std::string ensure_trailing_slash(const std::string& value) {
    if (value.empty()) {
        return "/";
    }
    if (value.back() == '/') {
        return value;
    }
    return value + "/";
}

std::string url_join(const std::string& base, const std::string& route) {
    std::string b = base;
    if (b.empty()) {
        return route;
    }
    while (!b.empty() && b.back() == '/') {
        b.pop_back();
    }
    std::string r = route;
    if (!r.empty() && r.front() != '/') {
        r = "/" + r;
    }
    return b + r;
}

std::string url_decode(const std::string& input) {
    auto hex_value = [](char ch) -> int {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' + 10;
        }
        if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        }
        return -1;
    };

    std::string out;
    out.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        const char ch = input[i];
        if (ch == '+') {
            out.push_back(' ');
            continue;
        }
        if (ch == '%' && i + 2 < input.size()) {
            const int hi = hex_value(input[i + 1]);
            const int lo = hex_value(input[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        out.push_back(ch);
    }
    return out;
}

std::string guess_mime_type(const std::filesystem::path& file) {
    const auto ext = to_lower(file.extension().string());
    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".js") return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".xml") return "application/xml; charset=utf-8";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".webp") return "image/webp";
    if (ext == ".txt") return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

}  // namespace blogpp
