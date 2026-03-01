#include "core/theme_extension_registry.h"

#include "core/utils.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace blogpp {

namespace {

bool inject_before(std::string& html, const std::string& marker, const std::string& fragment) {
    const auto pos = html.rfind(marker);
    if (pos == std::string::npos) {
        return false;
    }
    html.insert(pos, fragment);
    return true;
}

std::string strip_html_tags(const std::string& html) {
    std::string text;
    text.reserve(html.size());
    bool in_tag = false;
    for (char ch : html) {
        if (ch == '<') {
            in_tag = true;
            continue;
        }
        if (ch == '>') {
            in_tag = false;
            text.push_back(' ');
            continue;
        }
        if (!in_tag) {
            text.push_back(ch);
        }
    }
    return text;
}

size_t count_words(const std::string& text) {
    size_t words = 0;
    bool in_word = false;
    for (unsigned char ch : text) {
        if (std::isalnum(ch)) {
            if (!in_word) {
                ++words;
                in_word = true;
            }
        } else {
            in_word = false;
        }
    }
    return words;
}

size_t count_heading_tags(const std::string& html) {
    size_t count = 0;
    size_t pos = 0;
    while ((pos = html.find("<h", pos)) != std::string::npos) {
        if (pos + 2 < html.size() && std::isdigit(static_cast<unsigned char>(html[pos + 2]))) {
            ++count;
        }
        ++pos;
    }
    return count;
}

class PostInsightExtension : public ThemeRenderExtension {
public:
    std::string name() const override {
        return "post_insight";
    }

    void apply(const ThemeRenderContext& context, std::string& html) const override {
        if (context.template_name != "post") {
            return;
        }

        const auto content_it = context.values.find("content");
        if (content_it == context.values.end()) {
            return;
        }

        const auto words = count_words(strip_html_tags(content_it->second));
        const auto minutes = std::max<size_t>(1, (words + 219) / 220);
        const auto headings = count_heading_tags(content_it->second);

        std::string fragment =
            "<p class=\"post-insight\">" + std::to_string(minutes) + " min read";
        if (headings > 0) {
            fragment += " · " + std::to_string(headings) + " sections";
        }
        fragment += "</p>";

        if (!inject_before(html, "</header>", fragment)) {
            inject_before(html, "<section class=\"post-content\">", fragment);
        }
    }
};

}  // namespace

BLOGPP_REGISTER_THEME_EXTENSION("*", PostInsightExtension);

}  // namespace blogpp
