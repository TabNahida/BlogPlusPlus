#include "core/theme_extension_registry.h"

#include <string>

namespace blogpp {

namespace {

bool inject_body_attr(std::string& html, const std::string& attr) {
    const auto body_pos = html.find("<body");
    if (body_pos == std::string::npos) {
        return false;
    }
    const auto end = html.find('>', body_pos);
    if (end == std::string::npos) {
        return false;
    }
    html.insert(end, " " + attr);
    return true;
}

class AuroraRuntimeExtension : public ThemeRenderExtension {
public:
    std::string name() const override {
        return "aurora_runtime";
    }

    void apply(const ThemeRenderContext& context, std::string& html) const override {
        if (context.template_name == "layout") {
            inject_body_attr(html, "data-theme-runtime=\"aurora\"");
            return;
        }
        if (context.template_name == "home") {
            const std::string badge = "<p class=\"kicker\">Runtime: aurora.cpp</p>";
            const auto pos = html.find("</section>");
            if (pos != std::string::npos) {
                html.insert(pos, badge);
            }
        }
    }
};

}  // namespace

void ensure_aurora_theme_runtime_linked() {}

BLOGPP_REGISTER_THEME_EXTENSION("aurora", AuroraRuntimeExtension);

}  // namespace blogpp
