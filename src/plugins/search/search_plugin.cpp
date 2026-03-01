#include "plugins/search/search_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <sstream>

namespace blogpp {

std::string SearchPlugin::name() const {
    return "search";
}

void SearchPlugin::apply(SiteBuilder& builder) {
    const auto& site = builder.context();

    std::ostringstream json;
    json << "[\n";
    for (size_t i = 0; i < site.posts.size(); ++i) {
        const auto& post = site.posts[i];
        json << "  {\n"
             << "    \"title\": \"" << escape_json(post.title) << "\",\n"
             << "    \"url\": \"" << escape_json(post.route) << "\",\n"
             << "    \"summary\": \"" << escape_json(post.summary) << "\",\n"
             << "    \"date\": \"" << escape_json(post.date) << "\",\n"
             << "    \"tags\": [";
        for (size_t j = 0; j < post.tags.size(); ++j) {
            if (j > 0) json << ", ";
            json << "\"" << escape_json(post.tags[j]) << "\"";
        }
        json << "]\n"
             << "  }";
        if (i + 1 < site.posts.size()) {
            json << ",";
        }
        json << "\n";
    }
    json << "]\n";

    builder.add_generated("/search-index.json", json.str(), "application/json");

    const std::string body =
        "<section class=\"search-page\">"
        "<h1>Search</h1>"
        "<p>Search by title, summary, or tags.</p>"
        "<input id=\"search-input\" placeholder=\"Type to search...\" />"
        "<ul id=\"search-results\" class=\"post-list\"></ul>"
        "<script>"
        "const input=document.getElementById('search-input');"
        "const results=document.getElementById('search-results');"
        "let data=[];"
        "fetch('/search-index.json').then(r=>r.json()).then(v=>{"
        "data=(v||[]).map(item=>({"
        "...item,"
        "_search:[item.title,item.summary,(item.tags||[]).join(' ')].join(' ').toLowerCase()"
        "}));"
        "});"
        "function render(items){results.innerHTML=items.map(item=>"
        "`<li><a href=\"${item.url}\">${item.title}</a><span class=\"meta\">${item.date}</span><p>${item.summary}</p></li>`"
        ").join('');}"
        "let timer=0;"
        "input.addEventListener('input',()=>{"
        "clearTimeout(timer);"
        "timer=setTimeout(()=>{"
        "const q=input.value.trim().toLowerCase();"
        "if(!q){results.innerHTML='';return;}"
        "const items=data.filter(item=>item._search.includes(q));"
        "render(items);"
        "},90);"
        "});"
        "</script>"
        "</section>";

    builder.add_generated("/search/", builder.render_layout_page("Search", body, "Site search"), "text/html");
}

}  // namespace blogpp
