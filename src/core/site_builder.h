#pragma once

#include "core/config.h"
#include "core/content_loader.h"
#include "core/http_types.h"
#include "core/theme.h"
#include "core/types.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace blogpp {

class ServerPlugin;

class SiteBuilder {
public:
    explicit SiteBuilder(std::filesystem::path project_root);

    bool build();
    bool serve(int port);
    bool create_post(const std::string& title);

    SiteContext& context();
    const SiteContext& context() const;

    std::string site_title() const;
    std::string site_description() const;
    std::string base_url() const;
    std::string config_value(const std::string& key, const std::string& fallback = "") const;
    bool config_flag(const std::string& key, bool fallback = false) const;
    int config_int(const std::string& key, int fallback = 0) const;

    std::string render_layout_page(const std::string& page_title,
                                   const std::string& body_html,
                                   const std::string& description = "") const;
    std::string render_list_page(const std::string& title,
                                 const std::string& description,
                                 const std::string& items_html) const;

    void add_generated(const std::string& route,
                       const std::string& content,
                       const std::string& mime_type = "text/html");
    void register_route(const std::string& route);

private:
    std::filesystem::path project_root_;
    Config config_;
    ThemeEngine theme_;
    SiteContext site_;
    ContentLoader loader_;

    bool load_config();
    bool load_theme();
    bool prepare_output() const;
    void prepare_context();
    void render_core_pages();
    void run_static_plugins();
    std::vector<std::unique_ptr<ServerPlugin>> build_server_plugins() const;
    void write_output_files() const;
    void write_route_file(const OutputFile& file) const;
    void copy_theme_assets() const;
    void copy_static_assets() const;
    std::filesystem::path output_dir() const;
    std::string build_head_extra() const;
    std::string build_body_extra() const;
    std::string build_nav_html() const;
    std::string render_post_tags(const ContentItem& post) const;
    std::string render_post_card(const ContentItem& post) const;
};

}  // namespace blogpp
