#include "core/content_loader.h"

#include "core/utils.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace blogpp {

namespace {

struct FrontMatterResult {
    std::unordered_map<std::string, std::string> meta;
    std::string body;
};

FrontMatterResult parse_front_matter(const std::string& raw) {
    std::istringstream input(raw);
    std::string first_line;
    if (!std::getline(input, first_line)) {
        return {};
    }

    if (trim(first_line) != "---") {
        return {{}, raw};
    }

    std::unordered_map<std::string, std::string> meta;
    std::ostringstream body;
    std::string line;
    bool found_ending = false;

    while (std::getline(input, line)) {
        const auto cleaned = trim(line);
        if (cleaned == "---") {
            found_ending = true;
            break;
        }
        const auto pos = cleaned.find(':');
        if (pos == std::string::npos) {
            continue;
        }
        const auto key = trim(cleaned.substr(0, pos));
        auto value = trim(cleaned.substr(pos + 1));
        if (!value.empty() && ((value.front() == '"' && value.back() == '"') ||
                               (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }
        meta[key] = value;
    }

    if (!found_ending) {
        return {{}, raw};
    }

    while (std::getline(input, line)) {
        body << line << "\n";
    }
    return {meta, body.str()};
}

std::string derive_summary(const std::string& markdown) {
    std::istringstream input(markdown);
    std::string line;
    while (std::getline(input, line)) {
        auto cleaned = trim(line);
        if (cleaned.empty()) {
            continue;
        }
        if (cleaned.rfind("#", 0) == 0) {
            cleaned = trim(cleaned.substr(cleaned.find_first_not_of('#')));
        }
        if (cleaned.size() > 140) {
            cleaned = cleaned.substr(0, 140) + "...";
        }
        return cleaned;
    }
    return "No summary";
}

}  // namespace

ContentLoader::ContentLoader(MarkdownParser parser)
    : parser_(std::move(parser)) {}

void ContentLoader::load(const fs::path& content_dir, SiteContext& site) const {
    site.posts.clear();
    site.pages.clear();

    load_dir(content_dir / "posts", true, site);
    load_dir(content_dir / "pages", false, site);

    std::sort(site.posts.begin(), site.posts.end(), [](const ContentItem& lhs, const ContentItem& rhs) {
        if (lhs.date == rhs.date) {
            return lhs.title < rhs.title;
        }
        return lhs.date > rhs.date;
    });
}

void ContentLoader::load_dir(const fs::path& directory, bool is_post, SiteContext& site) const {
    if (!fs::exists(directory)) {
        return;
    }

    std::vector<fs::path> markdown_files;
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (to_lower(entry.path().extension().string()) != ".md") {
            continue;
        }
        markdown_files.push_back(entry.path());
    }

    if (is_post) {
        site.posts.reserve(site.posts.size() + markdown_files.size());
    } else {
        site.pages.reserve(site.pages.size() + markdown_files.size());
    }

    for (const auto& file : markdown_files) {
        auto item = parse_markdown_file(file, is_post);
        if (item.title.empty()) {
            continue;
        }
        if (is_post) {
            site.posts.push_back(std::move(item));
        } else {
            site.pages.push_back(std::move(item));
        }
    }
}

ContentItem ContentLoader::parse_markdown_file(const fs::path& file, bool is_post) const {
    ContentItem item;
    item.source_path = file.string();
    item.is_post = is_post;

    const auto raw = read_text_file(file);
    const auto parsed = parse_front_matter(raw);
    item.markdown = parsed.body;
    item.title = parsed.meta.count("title") ? parsed.meta.at("title") : file.stem().string();
    item.slug = parsed.meta.count("slug") ? slugify(parsed.meta.at("slug")) : slugify(item.title);
    item.date = parsed.meta.count("date") ? parsed.meta.at("date") : current_date();
    item.summary = parsed.meta.count("summary") ? parsed.meta.at("summary") : derive_summary(item.markdown);
    item.tags = parsed.meta.count("tags") ? parse_csv(parsed.meta.at("tags")) : std::vector<std::string>{};

    if (item.slug.empty()) {
        item.slug = slugify(file.stem().string());
    }
    if (item.title.empty()) {
        item.title = file.stem().string();
    }

    item.html = parser_.to_html(item.markdown);
    if (is_post) {
        item.route = "/posts/" + item.slug + "/";
    } else if (item.slug == "index") {
        item.route = "/";
    } else {
        item.route = "/" + item.slug + "/";
    }
    return item;
}

}  // namespace blogpp
