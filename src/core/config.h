#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace blogpp {

class Config {
public:
    bool load(const std::filesystem::path& path);
    std::string get(const std::string& key, const std::string& fallback = "") const;
    bool get_bool(const std::string& key, bool fallback = false) const;
    int get_int(const std::string& key, int fallback = 0) const;
    std::vector<std::string> get_list(const std::string& key) const;
    const std::unordered_map<std::string, std::string>& values() const;

private:
    std::unordered_map<std::string, std::string> values_;
};

}  // namespace blogpp
