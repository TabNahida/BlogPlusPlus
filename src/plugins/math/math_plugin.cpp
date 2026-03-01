#include "plugins/math/math_plugin.h"

#include "core/site_builder.h"
#include "plugins/plugin_utils.h"

namespace blogpp {

std::string MathPlugin::name() const {
    return "math";
}

void MathPlugin::apply(SiteBuilder& builder) {
    const std::string head_fragment =
        "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.16.11/dist/katex.min.css\" "
        "crossorigin=\"anonymous\" />";

    const std::string body_fragment =
        "<script defer src=\"https://cdn.jsdelivr.net/npm/katex@0.16.11/dist/katex.min.js\" "
        "crossorigin=\"anonymous\"></script>"
        "<script defer src=\"https://cdn.jsdelivr.net/npm/katex@0.16.11/dist/contrib/auto-render.min.js\" "
        "crossorigin=\"anonymous\"></script>"
        "<script>"
        "window.addEventListener('DOMContentLoaded',function(){"
        "if(typeof renderMathInElement!=='function'){return;}"
        "renderMathInElement(document.body,{"
        "delimiters:["
        "{left:'$$',right:'$$',display:true},"
        "{left:'$',right:'$',display:false},"
        "{left:'\\\\(',right:'\\\\)',display:false},"
        "{left:'\\\\[',right:'\\\\]',display:true}"
        "]"
        "});"
        "});"
        "</script>";

    auto& generated = builder.context().generated;
    for (auto& file : generated) {
        if (file.mime_type != "text/html") {
            continue;
        }
        plugin_utils::inject_before(file.content, "</head>", head_fragment);
        if (!plugin_utils::inject_before(file.content, "</body>", body_fragment)) {
            plugin_utils::append_fallback(file.content, body_fragment);
        }
    }
}

}  // namespace blogpp
