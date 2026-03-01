#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace blogpp {

struct ThemeRenderContext {
    std::string theme_name;
    std::string template_name;
    const std::unordered_map<std::string, std::string>& values;
};

class ThemeRenderExtension {
public:
    virtual ~ThemeRenderExtension() = default;
    virtual std::string name() const = 0;
    virtual void apply(const ThemeRenderContext& context, std::string& html) const = 0;
};

using ThemeRenderExtensionFactory = std::function<std::unique_ptr<ThemeRenderExtension>()>;

class ThemeRenderRegistry {
public:
    static ThemeRenderRegistry& instance();

    void register_factory(const std::string& theme_name, ThemeRenderExtensionFactory factory);
    std::vector<std::unique_ptr<ThemeRenderExtension>> create_for_theme(const std::string& theme_name) const;

private:
    std::unordered_map<std::string, std::vector<ThemeRenderExtensionFactory>> factories_;
};

class ThemeRenderAutoRegister {
public:
    ThemeRenderAutoRegister(std::string theme_name, ThemeRenderExtensionFactory factory);
};

}  // namespace blogpp

#define BLOGPP_CONCAT_DETAIL(a, b) a##b
#define BLOGPP_CONCAT(a, b) BLOGPP_CONCAT_DETAIL(a, b)

#define BLOGPP_REGISTER_THEME_EXTENSION(theme_name_literal, extension_type)                                 \
    namespace {                                                                                              \
    const ::blogpp::ThemeRenderAutoRegister BLOGPP_CONCAT(kThemeRenderRegister_, __LINE__)(                \
        (theme_name_literal),                                                                               \
        []() -> std::unique_ptr<::blogpp::ThemeRenderExtension> {                                           \
            return std::make_unique<extension_type>();                                                       \
        });                                                                                                  \
    }
