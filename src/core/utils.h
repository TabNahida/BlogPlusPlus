#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace blogpp {

std::string ltrim(const std::string& input);
std::string rtrim(const std::string& input);
std::string trim(const std::string& input);
std::string to_lower(std::string value);
std::vector<std::string> split(const std::string& input, char delim);
std::vector<std::string> parse_csv(const std::string& input);
std::string join(const std::vector<std::string>& values, const std::string& delim);
std::string replace_all(std::string text, const std::string& from, const std::string& to);
std::string slugify(const std::string& input);
std::string html_escape(const std::string& input);
std::string escape_json(const std::string& input);
std::string read_text_file(const std::filesystem::path& file);
bool write_text_file(const std::filesystem::path& file, const std::string& content);
bool file_exists(const std::filesystem::path& file);
std::string current_date();
std::string current_datetime();
std::string ensure_trailing_slash(const std::string& value);
std::string url_join(const std::string& base, const std::string& route);
std::string url_decode(const std::string& input);
std::string guess_mime_type(const std::filesystem::path& file);

}  // namespace blogpp
