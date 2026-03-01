#pragma once

#include "core/markdown.h"
#include "core/types.h"

#include <filesystem>

namespace blogpp {

class ContentLoader {
public:
    explicit ContentLoader(MarkdownParser parser = MarkdownParser{});
    void load(const std::filesystem::path& content_dir, SiteContext& site) const;

private:
    MarkdownParser parser_;

    void load_dir(const std::filesystem::path& directory, bool is_post, SiteContext& site) const;
    ContentItem parse_markdown_file(const std::filesystem::path& file, bool is_post) const;
};

}  // namespace blogpp

