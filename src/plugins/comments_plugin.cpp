#include "plugins/comments_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"
#include "plugins/plugin_utils.h"

#include <sstream>

namespace blogpp {

std::string CommentsPlugin::name() const {
    return "comments";
}

void CommentsPlugin::apply(SiteBuilder& builder) {
    auto provider = to_lower(builder.config_value("comments_provider", "giscus"));
    const auto giscus_repo = builder.config_value("comments_giscus_repo", "");
    const auto giscus_repo_id = builder.config_value("comments_giscus_repo_id", "");
    const auto giscus_category = builder.config_value("comments_giscus_category", "Announcements");
    const auto giscus_category_id = builder.config_value("comments_giscus_category_id", "");
    const auto disqus_shortname = builder.config_value("comments_disqus_shortname", "");
    const auto disqus_shortname_js = replace_all(disqus_shortname, "'", "\\'");

    std::ostringstream section;
    section << "<section class=\"comments-shell\">"
            << "<h2>Comments</h2>";

    if (provider == "giscus" && !giscus_repo.empty() && !giscus_repo_id.empty() && !giscus_category_id.empty()) {
        section << "<div class=\"comments-note\">Powered by Giscus</div>"
                << "<script src=\"https://giscus.app/client.js\" "
                << "data-repo=\"" << html_escape(giscus_repo) << "\" "
                << "data-repo-id=\"" << html_escape(giscus_repo_id) << "\" "
                << "data-category=\"" << html_escape(giscus_category) << "\" "
                << "data-category-id=\"" << html_escape(giscus_category_id) << "\" "
                << "data-mapping=\"pathname\" "
                << "data-strict=\"0\" data-reactions-enabled=\"1\" data-emit-metadata=\"0\" "
                << "data-input-position=\"bottom\" data-theme=\"preferred_color_scheme\" "
                << "crossorigin=\"anonymous\" async></script>";
    } else if (provider == "disqus" && !disqus_shortname.empty()) {
        section << "<div id=\"disqus_thread\"></div>"
                << "<script>"
                << "window.disqus_config=function(){this.page.url=window.location.href;this.page.identifier=window.location.pathname;};"
                << "(function(){var d=document,s=d.createElement('script');"
                << "s.src='https://" << disqus_shortname_js << ".disqus.com/embed.js';"
                << "s.setAttribute('data-timestamp',Date.now().toString());"
                << "(d.head||d.body).appendChild(s);}());"
                << "</script>";
    } else {
        section << "<p class=\"comments-note\">Comments plugin is enabled. Configure provider settings in blogpp.yml.</p>";
    }

    section << "</section>";
    const auto fragment = section.str();

    auto& generated = builder.context().generated;
    for (auto& file : generated) {
        if (!plugin_utils::route_starts_with(file, "/posts/")) {
            continue;
        }
        if (!plugin_utils::inject_before(file.content, "</article>", fragment)) {
            if (!plugin_utils::inject_before(file.content, "</main>", fragment)) {
                plugin_utils::append_fallback(file.content, fragment);
            }
        }
    }
}

}  // namespace blogpp
