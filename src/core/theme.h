#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace blogpp {

class ThemeEngine {
public:
    bool load(const std::filesystem::path& theme_dir);
    std::string render(const std::string& template_name,
                       const std::unordered_map<std::string, std::string>& context) const;
    std::string wrap_layout(const std::string& page_title,
                            const std::string& site_title,
                            const std::string& nav_html,
                            const std::string& content_html,
                            const std::string& description,
                            const std::string& head_extra = "",
                            const std::string& body_extra = "") const;
    const std::filesystem::path& dir() const;

private:
    std::filesystem::path dir_;
    std::unordered_map<std::string, std::string> templates_;
};

}  // namespace blogpp
