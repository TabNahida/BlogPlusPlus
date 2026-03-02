#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace blogpp {

struct ContentItem {
    std::string source_path;
    std::string title;
    std::string slug;
    std::string date;
    std::string summary;
    std::unordered_map<std::string, std::string> meta;
    std::vector<std::string> authors;
    std::vector<std::string> tags;
    std::string markdown;
    std::string html;
    std::string route;
    bool is_post = true;
};

struct OutputFile {
    std::string route;
    std::string content;
    std::string mime_type = "text/html";
};

struct SiteContext {
    std::unordered_map<std::string, std::string> config;
    std::vector<ContentItem> posts;
    std::vector<ContentItem> pages;
    std::vector<OutputFile> generated;
    std::vector<std::string> routes;
    std::vector<std::string> enabled_plugins;
    std::vector<std::string> enabled_server_plugins;
};

}  // namespace blogpp
