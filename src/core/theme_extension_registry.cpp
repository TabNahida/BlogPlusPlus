#include "core/theme_extension_registry.h"

#include "core/utils.h"

namespace blogpp {

ThemeRenderRegistry& ThemeRenderRegistry::instance() {
    static ThemeRenderRegistry registry;
    return registry;
}

void ThemeRenderRegistry::register_factory(const std::string& theme_name, ThemeRenderExtensionFactory factory) {
    const auto key = to_lower(trim(theme_name));
    if (key.empty()) {
        return;
    }
    factories_[key].push_back(std::move(factory));
}

std::vector<std::unique_ptr<ThemeRenderExtension>> ThemeRenderRegistry::create_for_theme(const std::string& theme_name) const {
    std::vector<std::unique_ptr<ThemeRenderExtension>> extensions;
    const auto key = to_lower(trim(theme_name));

    auto append_by_key = [&](const std::string& lookup) {
        const auto it = factories_.find(lookup);
        if (it == factories_.end()) {
            return;
        }
        for (const auto& factory : it->second) {
            if (!factory) {
                continue;
            }
            auto extension = factory();
            if (extension) {
                extensions.push_back(std::move(extension));
            }
        }
    };

    append_by_key("*");
    if (!key.empty()) {
        append_by_key(key);
    }
    return extensions;
}

ThemeRenderAutoRegister::ThemeRenderAutoRegister(std::string theme_name, ThemeRenderExtensionFactory factory) {
    ThemeRenderRegistry::instance().register_factory(std::move(theme_name), std::move(factory));
}

}  // namespace blogpp
