#include "core/markdown.h"

#include "core/utils.h"

#include <regex>
#include <sstream>

namespace blogpp {

namespace {

const std::regex kImagePattern(R"(!\[([^\]]*)\]\(([^)]+)\))");
const std::regex kLinkPattern(R"(\[([^\]]+)\]\(([^)]+)\))");
const std::regex kInlineCodePattern(R"(`([^`]+)`)");
const std::regex kStrongPattern(R"(\*\*([^*]+)\*\*)");
const std::regex kEmPattern(R"(\*([^*]+)\*)");

const std::regex kHeaderPattern(R"(^(#{1,6})\s+(.+)$)");
const std::regex kQuotePattern(R"(^>\s?(.*)$)");
const std::regex kUnorderedPattern(R"(^[-*]\s+(.+)$)");
const std::regex kOrderedPattern(R"(^\d+\.\s+(.+)$)");

}  // namespace

std::string MarkdownParser::render_inline(const std::string& input) const {
    std::string text = html_escape(input);
    text = std::regex_replace(text, kImagePattern, R"(<img alt="$1" src="$2" />)");
    text = std::regex_replace(text, kLinkPattern, R"(<a href="$2">$1</a>)");
    text = std::regex_replace(text, kInlineCodePattern, R"(<code>$1</code>)");
    text = std::regex_replace(text, kStrongPattern, R"(<strong>$1</strong>)");
    text = std::regex_replace(text, kEmPattern, R"(<em>$1</em>)");
    return text;
}

std::string MarkdownParser::to_html(const std::string& markdown) const {
    std::istringstream input(markdown);
    std::ostringstream out;

    bool in_code = false;
    std::string list_mode;
    std::string line;

    auto close_list = [&]() {
        if (list_mode == "ul") out << "</ul>\n";
        if (list_mode == "ol") out << "</ol>\n";
        list_mode.clear();
    };

    while (std::getline(input, line)) {
        std::string trimmed = trim(line);

        if (trimmed.rfind("```", 0) == 0) {
            close_list();
            if (!in_code) {
                std::string lang = trim(trimmed.substr(3));
                if (lang.empty()) {
                    out << "<pre><code>";
                } else {
                    out << "<pre><code class=\"language-" << html_escape(lang) << "\">";
                }
                in_code = true;
            } else {
                out << "</code></pre>\n";
                in_code = false;
            }
            continue;
        }

        if (in_code) {
            out << html_escape(line) << "\n";
            continue;
        }

        if (trimmed.empty()) {
            close_list();
            continue;
        }

        std::smatch match;
        if (std::regex_match(trimmed, match, kHeaderPattern)) {
            close_list();
            const int level = static_cast<int>(match[1].str().size());
            out << "<h" << level << ">" << render_inline(match[2].str()) << "</h" << level << ">\n";
            continue;
        }

        if (trimmed == "---" || trimmed == "***") {
            close_list();
            out << "<hr />\n";
            continue;
        }

        if (std::regex_match(trimmed, match, kQuotePattern)) {
            close_list();
            out << "<blockquote><p>" << render_inline(match[1].str()) << "</p></blockquote>\n";
            continue;
        }

        if (std::regex_match(trimmed, match, kUnorderedPattern)) {
            if (list_mode != "ul") {
                close_list();
                out << "<ul>\n";
                list_mode = "ul";
            }
            out << "<li>" << render_inline(match[1].str()) << "</li>\n";
            continue;
        }

        if (std::regex_match(trimmed, match, kOrderedPattern)) {
            if (list_mode != "ol") {
                close_list();
                out << "<ol>\n";
                list_mode = "ol";
            }
            out << "<li>" << render_inline(match[1].str()) << "</li>\n";
            continue;
        }

        close_list();
        out << "<p>" << render_inline(trimmed) << "</p>\n";
    }

    if (in_code) {
        out << "</code></pre>\n";
    }
    if (!list_mode.empty()) {
        close_list();
    }
    return out.str();
}

}  // namespace blogpp
