#include "plugins/rss_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <algorithm>
#include <sstream>

namespace blogpp {

std::string RssPlugin::name() const {
    return "rss";
}

void RssPlugin::apply(SiteBuilder& builder) {
    const auto& site = builder.context();
    if (site.posts.empty()) {
        return;
    }

    const auto base = builder.base_url();
    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<rss version=\"2.0\">\n"
        << "<channel>\n"
        << "<title>" << html_escape(builder.site_title()) << "</title>\n"
        << "<description>" << html_escape(builder.site_description()) << "</description>\n"
        << "<link>" << html_escape(base) << "</link>\n";

    const size_t limit = std::min<size_t>(site.posts.size(), 20);
    for (size_t i = 0; i < limit; ++i) {
        const auto& post = site.posts[i];
        xml << "<item>\n"
            << "<title>" << html_escape(post.title) << "</title>\n"
            << "<description>" << html_escape(post.summary) << "</description>\n"
            << "<link>" << html_escape(url_join(base, post.route)) << "</link>\n"
            << "<guid>" << html_escape(url_join(base, post.route)) << "</guid>\n"
            << "<pubDate>" << html_escape(post.date) << "</pubDate>\n"
            << "</item>\n";
    }

    xml << "</channel>\n</rss>\n";
    builder.add_generated("/feed.xml", xml.str(), "application/rss+xml");
}

}  // namespace blogpp

