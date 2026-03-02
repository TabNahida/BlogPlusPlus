#include "core/config.h"

#include "core/utils.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace blogpp {

namespace {

std::string strip_inline_comment(const std::string& line) {
    bool in_single = false;
    bool in_double = false;
    bool escaped = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (escaped) {
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '\'' && !in_double) {
            in_single = !in_single;
            continue;
        }
        if (ch == '"' && !in_single) {
            in_double = !in_double;
            continue;
        }
        if (ch == '#' && !in_single && !in_double) {
            if (i == 0 || std::isspace(static_cast<unsigned char>(line[i - 1]))) {
                return rtrim(line.substr(0, i));
            }
        }
    }
    return line;
}

std::string unquote(std::string value) {
    value = trim(value);
    if (value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

int leading_indent(const std::string& line) {
    int count = 0;
    for (char ch : line) {
        if (ch == ' ' || ch == '\t') {
            ++count;
            continue;
        }
        break;
    }
    return count;
}

void set_value(std::unordered_map<std::string, std::string>& values,
               std::vector<std::string>& key_order,
               const std::string& key,
               std::string value) {
    if (key.empty()) {
        return;
    }
    value = trim(std::move(value));
    if (values.find(key) == values.end()) {
        key_order.push_back(key);
    }
    values[key] = value;
}

void append_list_value(std::unordered_map<std::string, std::string>& values,
                       std::vector<std::string>& key_order,
                       const std::string& key,
                       const std::string& value) {
    const auto item = trim(value);
    if (key.empty() || item.empty()) {
        return;
    }
    auto it = values.find(key);
    if (it == values.end()) {
        key_order.push_back(key);
        values[key] = item;
        return;
    }
    if (!it->second.empty()) {
        it->second += ", ";
    }
    it->second += item;
}

}  // namespace

bool Config::load(const std::filesystem::path& path) {
    key_order_.clear();
    values_.clear();
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    struct ParentKey {
        int indent = 0;
        std::string full_key;
    };
    std::vector<ParentKey> stack;

    std::string line;
    while (std::getline(in, line)) {
        auto cleaned = trim(strip_inline_comment(line));
        if (cleaned.empty()) {
            continue;
        }

        const int indent = leading_indent(line);
        while (!stack.empty() && indent <= stack.back().indent) {
            stack.pop_back();
        }

        if (cleaned.rfind("- ", 0) == 0) {
            if (stack.empty()) {
                continue;
            }
            append_list_value(values_, key_order_, stack.back().full_key, unquote(cleaned.substr(2)));
            continue;
        }

        const auto pos = cleaned.find(':');
        if (pos == std::string::npos) {
            continue;
        }
        const auto key = trim(cleaned.substr(0, pos));
        if (key.empty()) {
            continue;
        }
        auto value = trim(cleaned.substr(pos + 1));

        std::string full_key = key;
        if (!stack.empty()) {
            full_key = stack.back().full_key + "." + key;
        }

        if (value.empty()) {
            set_value(values_, key_order_, full_key, "");
            stack.push_back({indent, full_key});
            continue;
        }

        set_value(values_, key_order_, full_key, unquote(value));
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

std::string Config::get_any(const std::vector<std::string>& keys, const std::string& fallback) const {
    for (const auto& key : keys) {
        const auto value = get(key, "");
        if (!trim(value).empty()) {
            return value;
        }
    }
    return fallback;
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

bool Config::get_bool_any(const std::vector<std::string>& keys, bool fallback) const {
    for (const auto& key : keys) {
        const auto value = trim(get(key, ""));
        if (!value.empty()) {
            return get_bool(key, fallback);
        }
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

int Config::get_int_any(const std::vector<std::string>& keys, int fallback) const {
    for (const auto& key : keys) {
        const auto value = trim(get(key, ""));
        if (!value.empty()) {
            return get_int(key, fallback);
        }
    }
    return fallback;
}

std::vector<std::string> Config::get_list(const std::string& key) const {
    return parse_csv(get(key, ""));
}

std::vector<std::string> Config::get_list_any(const std::vector<std::string>& keys) const {
    for (const auto& key : keys) {
        const auto value = trim(get(key, ""));
        if (!value.empty()) {
            return parse_csv(value);
        }
    }
    return {};
}

std::vector<std::pair<std::string, std::string>> Config::get_prefixed(const std::string& prefix) const {
    std::vector<std::pair<std::string, std::string>> items;
    if (prefix.empty()) {
        return items;
    }
    const std::string pattern = prefix + ".";
    for (const auto& key : key_order_) {
        if (key.rfind(pattern, 0) != 0) {
            continue;
        }
        const auto suffix = key.substr(pattern.size());
        if (suffix.empty()) {
            continue;
        }
        const auto value = trim(get(key, ""));
        if (value.empty()) {
            continue;
        }
        items.emplace_back(suffix, value);
    }
    return items;
}

const std::unordered_map<std::string, std::string>& Config::values() const {
    return values_;
}

}  // namespace blogpp
