#pragma once

#include <string>

namespace blogpp {

class MarkdownParser {
public:
    std::string to_html(const std::string& markdown) const;

private:
    std::string render_inline(const std::string& input) const;
};

}  // namespace blogpp

