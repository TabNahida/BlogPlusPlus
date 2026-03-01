#include "plugins/tags/tag_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <map>
#include <sstream>
#include <vector>

namespace blogpp {

std::string TagPlugin::name() const {
    return "tags";
}

void TagPlugin::apply(SiteBuilder& builder) {
    const auto& site = builder.context();
    if (site.posts.empty()) {
        return;
    }

    std::map<std::string, std::vector<const ContentItem*>> groups;
    for (const auto& post : site.posts) {
        if (post.tags.empty()) {
            groups["untagged"].push_back(&post);
            continue;
        }
        for (const auto& tag : post.tags) {
            groups[tag].push_back(&post);
        }
    }

    if (groups.empty()) {
        return;
    }

    std::ostringstream tag_index_items;
    tag_index_items << "<ul class=\"tag-index\">";

    for (const auto& [tag, posts] : groups) {
        const auto tag_slug = slugify(tag);
        const auto tag_route = "/tags/" + tag_slug + "/";

        std::ostringstream list_items;
        list_items << "<ul class=\"post-list\">";
        for (const auto* post : posts) {
            list_items << "<li><a href=\"" << post->route << "\">" << html_escape(post->title)
                       << "</a><span class=\"meta\">" << html_escape(post->date) << "</span></li>";
        }
        list_items << "</ul>";

        const auto page = builder.render_list_page("Tag: " + tag,
                                                   "Posts tagged with #" + tag,
                                                   list_items.str());
        builder.add_generated(tag_route, page, "text/html");

        tag_index_items << "<li><a href=\"" << tag_route << "\">#" << html_escape(tag)
                        << "</a><span class=\"count\">" << posts.size() << "</span></li>";
    }

    tag_index_items << "</ul>";
    builder.add_generated("/tags/",
                          builder.render_list_page("Tags", "Browse posts by tags", tag_index_items.str()),
                          "text/html");
}

}  // namespace blogpp
