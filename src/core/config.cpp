#include "core/config.h"

#include "core/utils.h"

#include <fstream>
#include <sstream>

namespace blogpp {

bool Config::load(const std::filesystem::path& path) {
    values_.clear();
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        auto cleaned = trim(line);
        if (cleaned.empty()) {
            continue;
        }
        if (cleaned.rfind("#", 0) == 0) {
            continue;
        }

        const auto pos = cleaned.find(':');
        if (pos == std::string::npos) {
            continue;
        }
        const auto key = trim(cleaned.substr(0, pos));
        auto value = trim(cleaned.substr(pos + 1));
        if (!value.empty() && ((value.front() == '"' && value.back() == '"') ||
                               (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }
        values_[key] = value;
    }
    return true;
}

std::string Config::get(const std::string& key, const std::string& fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return fallback;
    }
    return it->second;
}

bool Config::get_bool(const std::string& key, bool fallback) const {
    const auto raw = to_lower(trim(get(key, fallback ? "true" : "false")));
    if (raw == "1" || raw == "true" || raw == "yes" || raw == "on") {
        return true;
    }
    if (raw == "0" || raw == "false" || raw == "no" || raw == "off") {
        return false;
    }
    return fallback;
}

int Config::get_int(const std::string& key, int fallback) const {
    const auto raw = trim(get(key, ""));
    if (raw.empty()) {
        return fallback;
    }
    try {
        return std::stoi(raw);
    } catch (...) {
        return fallback;
    }
}

std::vector<std::string> Config::get_list(const std::string& key) const {
    return parse_csv(get(key, ""));
}

const std::unordered_map<std::string, std::string>& Config::values() const {
    return values_;
}

}  // namespace blogpp
