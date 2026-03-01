#include "plugins/cloud/cloud_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <filesystem>
#include <set>
#include <sstream>

namespace blogpp {

namespace {

std::string route_to_path(std::string route) {
    if (route.empty() || route == "/") {
        return "index.html";
    }
    if (!route.empty() && route.front() == '/') {
        route.erase(route.begin());
    }
    const std::filesystem::path route_path(route);
    if (route_path.has_extension()) {
        return route;
    }
    if (!route.empty() && route.back() != '/') {
        route.push_back('/');
    }
    return route + "index.html";
}

}  // namespace

std::string CloudPlugin::name() const {
    return "cloud";
}

void CloudPlugin::apply(SiteBuilder& builder) {
    const auto provider = builder.config_value("cloud_provider", "s3");
    const auto bucket = builder.config_value("cloud_bucket", "");
    const auto public_prefix = builder.config_value("cloud_public_prefix", "");

    std::set<std::string> unique_routes(builder.context().routes.begin(), builder.context().routes.end());

    std::ostringstream manifest;
    manifest << "{\n"
             << "  \"provider\": \"" << escape_json(provider) << "\",\n"
             << "  \"bucket\": \"" << escape_json(bucket) << "\",\n"
             << "  \"public_prefix\": \"" << escape_json(public_prefix) << "\",\n"
             << "  \"generated_at\": \"" << current_datetime() << "\",\n"
             << "  \"files\": [\n";

    size_t index = 0;
    for (const auto& route : unique_routes) {
        manifest << "    {\n"
                 << "      \"route\": \"" << escape_json(route) << "\",\n"
                 << "      \"path\": \"" << escape_json(route_to_path(route)) << "\"\n"
                 << "    }";
        if (++index < unique_routes.size()) {
            manifest << ",";
        }
        manifest << "\n";
    }
    manifest << "  ]\n"
             << "}\n";

    builder.add_generated("/cloud-sync-manifest.json", manifest.str(), "application/json");

    const std::string body =
        "<section class=\"cloud-page\">"
        "<p class=\"kicker\">Cloud</p>"
        "<h1>Cloud Drive Manifest</h1>"
        "<p class=\"desc\">Generated file: <code>/cloud-sync-manifest.json</code>.</p>"
        "<p>Provider: <strong>" +
        html_escape(provider) +
        "</strong></p>"
        "<p>Bucket: <strong>" +
        html_escape(bucket.empty() ? "(not set)" : bucket) +
        "</strong></p>"
        "<p class=\"meta\">Use this manifest in CI/CD scripts to sync generated files to OSS/S3/WebDAV drives.</p>"
        "</section>";

    builder.add_generated("/cloud/", builder.render_layout_page("Cloud", body, "Cloud drive sync manifest"), "text/html");
}

}  // namespace blogpp
