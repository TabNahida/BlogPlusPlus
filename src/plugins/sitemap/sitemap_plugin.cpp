#include "plugins/sitemap/sitemap_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <set>
#include <sstream>

namespace blogpp {

std::string SitemapPlugin::name() const {
    return "sitemap";
}

void SitemapPlugin::apply(SiteBuilder& builder) {
    const auto& site = builder.context();
    if (site.routes.empty()) {
        return;
    }

    std::set<std::string> unique_routes(site.routes.begin(), site.routes.end());
    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n";

    const auto base = builder.base_url();
    for (const auto& route : unique_routes) {
        xml << "  <url>\n"
            << "    <loc>" << html_escape(url_join(base, route)) << "</loc>\n"
            << "    <lastmod>" << current_date() << "</lastmod>\n"
            << "  </url>\n";
    }

    xml << "</urlset>\n";
    builder.add_generated("/sitemap.xml", xml.str(), "application/xml");
}

}  // namespace blogpp
