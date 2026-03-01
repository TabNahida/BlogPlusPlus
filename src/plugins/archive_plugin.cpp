#include "plugins/archive_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <functional>
#include <map>
#include <sstream>
#include <vector>

namespace blogpp {

std::string ArchivePlugin::name() const {
    return "archives";
}

void ArchivePlugin::apply(SiteBuilder& builder) {
    const auto& site = builder.context();
    if (site.posts.empty()) {
        return;
    }

    std::map<std::string, std::vector<const ContentItem*>, std::greater<>> grouped;
    for (const auto& post : site.posts) {
        const auto key = post.date.size() >= 7 ? post.date.substr(0, 7) : post.date;
        grouped[key].push_back(&post);
    }

    std::ostringstream sections;
    for (const auto& [month, posts] : grouped) {
        sections << "<section class=\"archive-group\"><h2>" << html_escape(month) << "</h2><ul class=\"post-list\">";
        for (const auto* post : posts) {
            sections << "<li><a href=\"" << post->route << "\">" << html_escape(post->title)
                     << "</a><span class=\"meta\">" << html_escape(post->date) << "</span></li>";
        }
        sections << "</ul></section>";
    }

    builder.add_generated("/archives/",
                          builder.render_list_page("Archives", "Browse posts by month", sections.str()),
                          "text/html");
}

}  // namespace blogpp

