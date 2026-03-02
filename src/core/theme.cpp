#include "core/theme.h"

#include "core/utils.h"

#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

namespace blogpp {

namespace {

std::unordered_map<std::string, std::string> fallback_templates() {
    return {
        {"layout",
         "<!doctype html>\n"
         "<html><head><meta charset=\"utf-8\" /><meta name=\"viewport\" "
         "content=\"width=device-width, initial-scale=1\" />"
         "<title>{{page_title}} | {{site_title}}</title>"
         "<meta name=\"description\" content=\"{{description}}\" />"
         "<link rel=\"stylesheet\" href=\"/assets/style.css\" />{{head_extra}}</head>"
         "<body><header><h1><a href=\"/\">{{site_title}}</a></h1><nav>{{nav}}</nav></header>"
         "<main>{{content}}</main><footer><small>{{year}} {{site_title}}</small>{{footer_extra}}</footer>{{body_extra}}</body></html>"},
        {"home", "<section><h2>Latest Posts</h2><div class=\"cards\">{{posts}}</div></section>"},
        {"post",
         "<article class=\"post\"><h1>{{title}}</h1><p class=\"meta\">{{date}} {{tags}}</p>"
         "<div class=\"post-content\">{{content}}</div></article>"},
        {"page", "<article class=\"page\"><h1>{{title}}</h1><div class=\"page-content\">{{content}}</div></article>"},
        {"list", "<section class=\"list\"><h1>{{title}}</h1><p>{{description}}</p><div>{{items}}</div></section>"},
    };
}

}  // namespace

bool ThemeEngine::load(const fs::path& theme_dir) {
    dir_ = theme_dir;
    theme_name_ = to_lower(theme_dir.filename().string());
    templates_ = fallback_templates();
    extensions_ = ThemeRenderRegistry::instance().create_for_theme(theme_name_);

    if (!fs::exists(theme_dir)) {
        return false;
    }

    for (const auto& entry : fs::directory_iterator(theme_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".html") {
            continue;
        }
        const auto key = entry.path().stem().string();
        templates_[key] = read_text_file(entry.path());
    }
    return true;
}

std::string ThemeEngine::render(const std::string& template_name,
                                const std::unordered_map<std::string, std::string>& context) const {
    auto it = templates_.find(template_name);
    if (it == templates_.end()) {
        return "";
    }
    auto content = it->second;
    for (const auto& [key, value] : context) {
        content = replace_all(content, "{{" + key + "}}", value);
    }

    // Clear unknown placeholders to avoid leaking template markers.
    size_t pos = 0;
    while ((pos = content.find("{{", pos)) != std::string::npos) {
        const auto end = content.find("}}", pos + 2);
        if (end == std::string::npos) {
            break;
        }
        content.erase(pos, end - pos + 2);
    }

    ThemeRenderContext render_context{theme_name_, template_name, context};
    for (const auto& extension : extensions_) {
        if (!extension) {
            continue;
        }
        extension->apply(render_context, content);
    }
    return content;
}

std::string ThemeEngine::wrap_layout(const std::string& page_title,
                                     const std::string& site_title,
                                     const std::string& nav_html,
                                     const std::string& content_html,
                                     const std::string& description,
                                     const std::string& head_extra,
                                     const std::string& body_extra,
                                     const std::string& footer_extra) const {
    return render(
        "layout",
        {
            {"page_title", html_escape(page_title)},
            {"site_title", html_escape(site_title)},
            {"nav", nav_html},
            {"content", content_html},
            {"description", html_escape(description)},
            {"head_extra", head_extra},
            {"body_extra", body_extra},
            {"footer_extra", footer_extra},
            {"year", current_date().substr(0, 4)},
        });
}

const fs::path& ThemeEngine::dir() const {
    return dir_;
}

}  // namespace blogpp
