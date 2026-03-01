#include "core/site_builder.h"

#include "core/build_flags.h"
#include "core/http_server.h"
#include "core/utils.h"
#include "plugins/plugin.h"
#include "plugins/plugin_registry.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <set>
#include <unordered_set>

namespace fs = std::filesystem;

namespace blogpp {

namespace {

std::string normalize_route(std::string route) {
    route = trim(route);
    if (route.empty()) {
        return "/";
    }
    if (route.front() != '/') {
        route = "/" + route;
    }
    const bool has_ext = fs::path(route).has_extension();
    if (!has_ext && route.back() != '/') {
        route.push_back('/');
    }
    return route;
}

std::string css_url_escape(std::string value) {
    value = replace_all(value, "\\", "\\\\");
    value = replace_all(value, "'", "\\'");
    value = replace_all(value, "\"", "\\\"");
    return value;
}

}  // namespace

SiteBuilder::SiteBuilder(fs::path project_root)
    : project_root_(std::move(project_root)) {}

bool SiteBuilder::build() {
    if (!load_config()) {
        return false;
    }
    prepare_context();
    if (!load_theme()) {
        return false;
    }
    if (!prepare_output()) {
        return false;
    }

    render_core_pages();
    run_static_plugins();
    write_output_files();
    copy_theme_assets();
    copy_static_assets();

    std::cout << "Build completed. Output: " << output_dir().string() << "\n";
    return true;
}

bool SiteBuilder::serve(int port) {
    if (!build()) {
        return false;
    }

#if BLOGPP_BUILD_FEATURE_SERVER
    int cache_mb = config_int("server_cache_mb", 32);
    if (cache_mb < 0) {
        cache_mb = 0;
    }
    const size_t cache_bytes = static_cast<size_t>(cache_mb) * 1024u * 1024u;

    auto server_plugins = build_server_plugins();
    StaticHttpServer server(output_dir(), cache_bytes);
    auto handler = [this, &server_plugins](const HttpRequest& request, HttpResponse& response) {
        for (auto& plugin : server_plugins) {
            if (plugin->handle(request, response, *this)) {
                return true;
            }
        }
        return false;
    };
    return server.start(port, handler);
#else
    (void)port;
    std::cerr << "Serve mode was disabled at build time. Rebuild with `xmake f --feature_server=y`.\n";
    return false;
#endif
}

bool SiteBuilder::create_post(const std::string& title) {
    const auto slug = slugify(title);
    const auto date = current_date();
    const auto filename = date + "-" + slug + ".md";
    const auto post_path = project_root_ / "content" / "posts" / filename;
    if (file_exists(post_path)) {
        std::cerr << "Post already exists: " << post_path.string() << "\n";
        return false;
    }

    const std::string template_text =
        "---\n"
        "title: " +
        title + "\n"
                "date: " +
        date + "\n"
               "tags: cpp, blog\n"
               "summary: Write your summary here.\n"
               "---\n\n"
               "# " +
        title + "\n\nStart writing here.\n";

    if (!write_text_file(post_path, template_text)) {
        std::cerr << "Failed to create post: " << post_path.string() << "\n";
        return false;
    }
    std::cout << "Created: " << post_path.string() << "\n";
    return true;
}

SiteContext& SiteBuilder::context() {
    return site_;
}

const SiteContext& SiteBuilder::context() const {
    return site_;
}

std::string SiteBuilder::site_title() const {
    return config_.get("title", "BlogPlusPlus");
}

std::string SiteBuilder::site_description() const {
    return config_.get("description", "Build by C++ with static export and server mode.");
}

std::string SiteBuilder::base_url() const {
    return config_.get("base_url", "http://localhost:4000");
}

std::string SiteBuilder::config_value(const std::string& key, const std::string& fallback) const {
    return config_.get(key, fallback);
}

bool SiteBuilder::config_flag(const std::string& key, bool fallback) const {
    return config_.get_bool(key, fallback);
}

int SiteBuilder::config_int(const std::string& key, int fallback) const {
    return config_.get_int(key, fallback);
}

std::string SiteBuilder::render_layout_page(const std::string& page_title,
                                            const std::string& body_html,
                                            const std::string& description) const {
    return theme_.wrap_layout(page_title,
                              site_title(),
                              build_nav_html(),
                              body_html,
                              description.empty() ? site_description() : description,
                              build_head_extra(),
                              build_body_extra());
}

std::string SiteBuilder::render_list_page(const std::string& title,
                                          const std::string& description,
                                          const std::string& items_html) const {
    const auto body = theme_.render("list",
                                    {
                                        {"title", html_escape(title)},
                                        {"description", html_escape(description)},
                                        {"items", items_html},
                                    });
    return render_layout_page(title, body, description);
}

void SiteBuilder::add_generated(const std::string& route,
                                const std::string& content,
                                const std::string& mime_type) {
    const auto normalized = normalize_route(route);
    auto it = std::find_if(site_.generated.begin(), site_.generated.end(), [&](const OutputFile& file) {
        return file.route == normalized;
    });
    if (it == site_.generated.end()) {
        site_.generated.push_back({normalized, content, mime_type});
    } else {
        it->content = content;
        it->mime_type = mime_type;
    }
    register_route(normalized);
}

void SiteBuilder::register_route(const std::string& route) {
    const auto normalized = normalize_route(route);
    if (std::find(site_.routes.begin(), site_.routes.end(), normalized) == site_.routes.end()) {
        site_.routes.push_back(normalized);
    }
}

bool SiteBuilder::load_config() {
    const auto cfg_path = project_root_ / "blogpp.yml";
    if (!config_.load(cfg_path)) {
        std::cerr << "Config file not found (" << cfg_path.string() << "), use defaults.\n";
    }
    return true;
}

bool SiteBuilder::load_theme() {
    const auto theme_name = config_.get("theme", "aurora");
    const auto path = project_root_ / "themes" / theme_name;
    if (theme_.load(path)) {
        return true;
    }
    std::cerr << "Theme not found: " << path.string() << ", using fallback templates.\n";
    return true;
}

bool SiteBuilder::prepare_output() const {
    const auto out = output_dir();
    std::error_code ec;
    if (fs::exists(out, ec)) {
        fs::remove_all(out, ec);
        if (ec) {
            std::cerr << "Failed to clean output directory: " << out.string() << "\n";
            return false;
        }
    }
    fs::create_directories(out, ec);
    if (ec) {
        std::cerr << "Failed to create output directory: " << out.string() << "\n";
        return false;
    }
    return true;
}

void SiteBuilder::prepare_context() {
    site_ = {};
    site_.config = config_.values();

    site_.enabled_plugins = config_.get_list("plugins");
    if (site_.enabled_plugins.empty()) {
        site_.enabled_plugins = {"tags", "archives", "rss", "search", "sitemap", "math", "comments", "forum", "cloud"};
    }

    site_.enabled_server_plugins = config_.get_list("server_plugins");
    if (site_.enabled_server_plugins.empty()) {
        site_.enabled_server_plugins = {"forum_api"};
    }

    loader_.load(project_root_ / "content", site_);
}

void SiteBuilder::render_core_pages() {
    for (const auto& post : site_.posts) {
        const auto body = theme_.render(
            "post",
            {
                {"title", html_escape(post.title)},
                {"date", html_escape(post.date)},
                {"summary", html_escape(post.summary)},
                {"tags", render_post_tags(post)},
                {"content", post.html},
            });
        add_generated(post.route, render_layout_page(post.title, body, post.summary), "text/html");
    }

    for (const auto& page : site_.pages) {
        const auto body = theme_.render(
            "page",
            {
                {"title", html_escape(page.title)},
                {"content", page.html},
            });
        add_generated(page.route, render_layout_page(page.title, body, page.summary), "text/html");
    }

    std::string cards;
    cards.reserve(site_.posts.size() * 220);
    for (const auto& post : site_.posts) {
        cards += render_post_card(post);
    }
    if (cards.empty()) {
        cards = "<p>No posts yet. Run <code>blogpp new \"My First Post\"</code> to start.</p>";
    }

    const auto home_body = theme_.render(
        "home",
        {
            {"site_title", html_escape(site_title())},
            {"intro", html_escape(site_description())},
            {"posts", cards},
        });
    add_generated("/", render_layout_page(site_title(), home_body, site_description()), "text/html");

    const auto not_found_body = theme_.render(
        "list",
        {
            {"title", "404"},
            {"description", "Page not found"},
            {"items", "<p>The page you requested does not exist.</p>"},
        });
    add_generated("/404/", render_layout_page("404", not_found_body, "Not found"), "text/html");
}

void SiteBuilder::run_static_plugins() {
    std::unordered_set<std::string> enabled;
    for (const auto& name : site_.enabled_plugins) {
        enabled.insert(to_lower(name));
    }

    auto plugins = create_static_plugins();

    for (auto& plugin : plugins) {
        if (enabled.find(to_lower(plugin->name())) == enabled.end()) {
            continue;
        }
        plugin->apply(*this);
    }
}

std::vector<std::unique_ptr<ServerPlugin>> SiteBuilder::build_server_plugins() const {
    std::unordered_set<std::string> enabled;
    for (const auto& name : site_.enabled_server_plugins) {
        enabled.insert(to_lower(name));
    }

    auto plugins = create_server_plugins();
    plugins.erase(std::remove_if(plugins.begin(),
                                 plugins.end(),
                                 [&](const std::unique_ptr<ServerPlugin>& plugin) {
                                     if (!plugin) {
                                         return true;
                                     }
                                     return enabled.find(to_lower(plugin->name())) == enabled.end();
                                 }),
                  plugins.end());
    return plugins;
}

void SiteBuilder::write_output_files() const {
    for (const auto& file : site_.generated) {
        write_route_file(file);
    }
}

void SiteBuilder::write_route_file(const OutputFile& file) const {
    std::string route = file.route;
    if (route.empty()) {
        route = "/";
    }
    if (!route.empty() && route.front() == '/') {
        route.erase(route.begin());
    }

    fs::path target = output_dir();
    if (route.empty()) {
        target /= "index.html";
    } else {
        const bool has_ext = fs::path(route).has_extension();
        if (!has_ext && route.back() != '/') {
            route.push_back('/');
        }
        if (!route.empty() && route.back() == '/') {
            target /= route;
            target /= "index.html";
        } else {
            target /= route;
        }
    }
    if (!write_text_file(target, file.content)) {
        std::cerr << "Failed to write file: " << target.string() << "\n";
    }
}

void SiteBuilder::copy_theme_assets() const {
    const auto theme_dir = theme_.dir();
    if (!fs::exists(theme_dir)) {
        return;
    }
    for (const auto& entry : fs::recursive_directory_iterator(theme_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto ext = to_lower(entry.path().extension().string());
        if (ext == ".html" || ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == ".h" ||
            ext == ".hpp" || ext == ".lua") {
            continue;
        }
        std::error_code ec;
        auto rel = fs::relative(entry.path(), theme_dir, ec);
        if (ec) {
            continue;
        }
        const auto target = output_dir() / "assets" / rel;
        fs::create_directories(target.parent_path(), ec);
        fs::copy_file(entry.path(), target, fs::copy_options::overwrite_existing, ec);
    }
}

void SiteBuilder::copy_static_assets() const {
    const auto static_dir = project_root_ / "content" / "static";
    if (!fs::exists(static_dir)) {
        return;
    }
    for (const auto& entry : fs::recursive_directory_iterator(static_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::error_code ec;
        auto rel = fs::relative(entry.path(), static_dir, ec);
        if (ec) {
            continue;
        }
        const auto target = output_dir() / rel;
        fs::create_directories(target.parent_path(), ec);
        fs::copy_file(entry.path(), target, fs::copy_options::overwrite_existing, ec);
    }
}

fs::path SiteBuilder::output_dir() const {
    return project_root_ / config_.get("output", "public");
}

std::string SiteBuilder::build_head_extra() const {
    std::string head_extra;
    const auto background_image = config_.get("background_image", "");
    if (!background_image.empty()) {
        head_extra += "<style>:root{--theme-background-image:url('";
        head_extra += css_url_escape(background_image);
        head_extra += "');}</style>";
    }
    return head_extra;
}

std::string SiteBuilder::build_body_extra() const {
    auto palettes = config_.get_list("theme_palettes");
    if (palettes.empty()) {
        palettes = {"ocean", "sunset", "forest"};
    }

    std::string array_text = "[";
    for (size_t i = 0; i < palettes.size(); ++i) {
        if (i > 0) {
            array_text += ",";
        }
        array_text += "\"";
        array_text += escape_json(palettes[i]);
        array_text += "\"";
    }
    array_text += "]";

    return "<script>"
           "(function(){"
           "const key='blogpp.palette';"
           "const palettes=" +
           array_text +
           ";"
           "if(!palettes.length){return;}"
           "const root=document.documentElement;"
           "const initial=localStorage.getItem(key)||palettes[0];"
           "root.setAttribute('data-palette',initial);"
           "let btn=document.querySelector('[data-theme-toggle]');"
           "if(!btn){"
           "btn=document.createElement('button');"
           "btn.type='button';"
           "btn.className='theme-toggle';"
           "btn.setAttribute('data-theme-toggle','1');"
           "document.body.appendChild(btn);"
           "}"
           "const renderLabel=(name)=>{btn.textContent='Palette: '+name;};"
           "renderLabel(initial);"
           "btn.addEventListener('click',function(){"
           "const current=root.getAttribute('data-palette')||palettes[0];"
           "const index=palettes.indexOf(current);"
           "const next=palettes[(index+1+palettes.length)%palettes.length];"
           "root.setAttribute('data-palette',next);"
           "localStorage.setItem(key,next);"
           "renderLabel(next);"
           "});"
           "})();"
           "</script>";
}

std::string SiteBuilder::build_nav_html() const {
    const auto nav_value =
        config_.get("nav", "Home:/,Archives:/archives/,Tags:/tags/,Search:/search/,Forum:/forum/,Cloud:/cloud/");
    const auto entries = parse_csv(nav_value);

    std::vector<std::string> links;
    for (const auto& entry : entries) {
        auto pos = entry.find('=');
        if (pos == std::string::npos) {
            pos = entry.find(':');
        }
        if (pos == std::string::npos) {
            continue;
        }
        const auto label = trim(entry.substr(0, pos));
        const auto url = trim(entry.substr(pos + 1));
        if (label.empty() || url.empty()) {
            continue;
        }
        links.push_back("<a href=\"" + html_escape(url) + "\">" + html_escape(label) + "</a>");
    }

    if (links.empty()) {
        links = {
            "<a href=\"/\">Home</a>",
            "<a href=\"/archives/\">Archives</a>",
            "<a href=\"/tags/\">Tags</a>",
            "<a href=\"/search/\">Search</a>",
            "<a href=\"/forum/\">Forum</a>",
        };
    }
    return join(links, "\n");
}

std::string SiteBuilder::render_post_tags(const ContentItem& post) const {
    if (post.tags.empty()) {
        return "<span class=\"tag muted\">untagged</span>";
    }
    std::vector<std::string> items;
    items.reserve(post.tags.size());
    for (const auto& tag : post.tags) {
        const auto tag_slug = slugify(tag);
        items.push_back("<a class=\"tag\" href=\"/tags/" + tag_slug + "/\">#" + html_escape(tag) + "</a>");
    }
    return join(items, " ");
}

std::string SiteBuilder::render_post_card(const ContentItem& post) const {
    return "<article class=\"post-card\">"
           "<h2><a href=\"" +
           post.route + "\">" + html_escape(post.title) +
           "</a></h2><p class=\"meta\">" + html_escape(post.date) +
           "</p><p class=\"summary\">" + html_escape(post.summary) +
           "</p><p class=\"tags\">" + render_post_tags(post) + "</p></article>";
}

}  // namespace blogpp
