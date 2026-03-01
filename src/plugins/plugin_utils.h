#pragma once

#include "core/types.h"

#include <string>

namespace blogpp::plugin_utils {

inline bool route_starts_with(const OutputFile& file, const std::string& prefix) {
    return file.mime_type == "text/html" && file.route.rfind(prefix, 0) == 0;
}

inline bool inject_before(std::string& html, const std::string& marker, const std::string& fragment) {
    const auto pos = html.rfind(marker);
    if (pos == std::string::npos) {
        return false;
    }
    html.insert(pos, fragment);
    return true;
}

inline void append_fallback(std::string& html, const std::string& fragment) {
    html += fragment;
}

}  // namespace blogpp::plugin_utils
