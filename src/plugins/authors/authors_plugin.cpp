#include "plugins/authors/authors_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <map>
#include <sstream>
#include <vector>

namespace blogpp {

std::string AuthorsPlugin::name() const {
    return "authors";
}

void AuthorsPlugin::apply(SiteBuilder& builder) {
    const auto& site = builder.context();
    if (site.posts.empty()) {
        return;
    }

    std::map<std::string, std::vector<const ContentItem*>> groups;
    for (const auto& post : site.posts) {
        if (post.authors.empty()) {
            groups["Anonymous"].push_back(&post);
            continue;
        }
        for (const auto& author : post.authors) {
            if (!trim(author).empty()) {
                groups[author].push_back(&post);
            }
        }
    }

    if (groups.empty()) {
        return;
    }

    std::ostringstream index_items;
    index_items << "<ul class=\"tag-index\">";

    for (const auto& [author, posts] : groups) {
        const auto author_slug = slugify(author);
        const auto route = "/authors/" + author_slug + "/";

        std::ostringstream list_items;
        list_items << "<ul class=\"post-list\">";
        for (const auto* post : posts) {
            list_items << "<li><a href=\"" << post->route << "\">" << html_escape(post->title)
                       << "</a><span class=\"meta\">" << html_escape(post->date) << "</span></li>";
        }
        list_items << "</ul>";

        builder.add_generated(route,
                              builder.render_list_page("Author: " + author,
                                                       "Posts by @" + author,
                                                       list_items.str()),
                              "text/html");

        index_items << "<li><a href=\"" << route << "\">@" << html_escape(author)
                    << "</a><span class=\"count\">" << posts.size() << "</span></li>";
    }

    index_items << "</ul>";
    builder.add_generated("/authors/",
                          builder.render_list_page("Authors", "Browse posts by author", index_items.str()),
                          "text/html");
}

}  // namespace blogpp
