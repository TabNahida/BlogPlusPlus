#include "plugins/forum_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <algorithm>

namespace blogpp {

std::string ForumPlugin::name() const {
    return "forum";
}

void ForumPlugin::apply(SiteBuilder& builder) {
    auto enabled_server_plugins = parse_csv(builder.config_value("server_plugins", "forum_api"));
    std::transform(enabled_server_plugins.begin(),
                   enabled_server_plugins.end(),
                   enabled_server_plugins.begin(),
                   [](std::string name) { return to_lower(trim(name)); });
    const bool forum_api_enabled =
        std::find(enabled_server_plugins.begin(), enabled_server_plugins.end(), "forum_api") != enabled_server_plugins.end();

    std::string api_hint;
    if (!forum_api_enabled) {
        api_hint =
            "<p class=\"desc\">Server API is disabled in config (`server_plugins`). "
            "Forum page can still be styled as a static landing page.</p>";
    }

    const std::string body =
        "<section class=\"forum-page\">"
        "<p class=\"kicker\">Community</p>"
        "<h1>Forum</h1>" +
        api_hint +
        "<p class=\"desc\">Build-time static page + runtime API plugin for local/server deployment.</p>"
        "<form id=\"forum-form\" class=\"forum-form\">"
        "<input id=\"forum-author\" name=\"author\" placeholder=\"Your name\" maxlength=\"40\" required />"
        "<textarea id=\"forum-text\" name=\"text\" rows=\"4\" maxlength=\"360\" "
        "placeholder=\"Share your idea...\" required></textarea>"
        "<button type=\"submit\">Post</button>"
        "</form>"
        "<ul id=\"forum-list\" class=\"post-list\"></ul>"
        "<script>"
        "const list=document.getElementById('forum-list');"
        "const form=document.getElementById('forum-form');"
        "async function load(){"
        "try{"
        "const res=await fetch('/api/forum/messages');"
        "if(!res.ok){throw new Error('api-offline');}"
        "const data=await res.json();"
        "list.innerHTML=(data||[]).map(item=>`<li><strong>${item.author}</strong>"
        "<span class=\\\"meta\\\">${item.created_at}</span><p>${item.text}</p></li>`).join('');"
        "}catch(_){"
        "list.innerHTML='<li><p class=\"meta\">Forum API unavailable. Start with `blogpp serve`.</p></li>';"
        "}"
        "}"
        "form.addEventListener('submit',async(e)=>{"
        "e.preventDefault();"
        "const author=(document.getElementById('forum-author').value||'').trim();"
        "const text=(document.getElementById('forum-text').value||'').trim();"
        "if(!author||!text){return;}"
        "try{"
        "const body=new URLSearchParams({author,text}).toString();"
        "const res=await fetch('/api/forum/messages',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});"
        "if(!res.ok){throw new Error('post-failed');}"
        "document.getElementById('forum-text').value='';"
        "await load();"
        "}catch(_){alert('Forum API unavailable or disabled.');}"
        "});"
        "load();"
        "</script>"
        "</section>";

    builder.add_generated("/forum/", builder.render_layout_page("Forum", body, "Community forum"), "text/html");
}

}  // namespace blogpp
