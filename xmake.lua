set_project("BlogPlusPlus")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")
set_languages("cxx20")

option("feature_server")
    set_default(true)
    set_showmenu(true)
    set_description("Enable builtin HTTP server mode")
option_end()

option("plugin_tags")
    set_default(true)
    set_showmenu(true)
    set_description("Enable tags plugin")
option_end()

option("plugin_archives")
    set_default(true)
    set_showmenu(true)
    set_description("Enable archives plugin")
option_end()

option("plugin_rss")
    set_default(true)
    set_showmenu(true)
    set_description("Enable RSS plugin")
option_end()

option("plugin_search")
    set_default(true)
    set_showmenu(true)
    set_description("Enable search plugin")
option_end()

option("plugin_sitemap")
    set_default(true)
    set_showmenu(true)
    set_description("Enable sitemap plugin")
option_end()

option("plugin_comments")
    set_default(true)
    set_showmenu(true)
    set_description("Enable comments plugin")
option_end()

option("plugin_math")
    set_default(true)
    set_showmenu(true)
    set_description("Enable markdown math rendering plugin")
option_end()

option("plugin_cloud")
    set_default(true)
    set_showmenu(true)
    set_description("Enable cloud drive export plugin")
option_end()

option("plugin_authors")
    set_default(true)
    set_showmenu(true)
    set_description("Enable author index plugin")
option_end()

option("plugin_forum")
    set_default(true)
    set_showmenu(true)
    set_description("Enable static forum page plugin")
option_end()

option("server_plugin_forum_api")
    set_default(true)
    set_showmenu(true)
    set_description("Enable server-side forum API plugin")
option_end()

option("plugin_post_protect")
    set_default(true)
    set_showmenu(true)
    set_description("Enable encrypted post static plugin")
option_end()

option("server_plugin_post_auth")
    set_default(true)
    set_showmenu(true)
    set_description("Enable server-side post unlock API")
option_end()

option("theme_aurora_runtime")
    set_default(true)
    set_showmenu(true)
    set_description("Enable aurora theme runtime extension")
option_end()

includes("src/plugins/forum/xmake.lua")
includes("src/plugins/forum_api/xmake.lua")
includes("src/plugins/post_protect/xmake.lua")
includes("src/plugins/post_auth_api/xmake.lua")
includes("themes/aurora/xmake.lua")

target("blogpp")
    set_kind("binary")
    add_includedirs("src")

    add_files(
        "src/main.cpp",
        "src/core/config.cpp",
        "src/core/content_loader.cpp",
        "src/core/markdown.cpp",
        "src/core/site_builder.cpp",
        "src/core/theme.cpp",
        "src/core/theme_extension_registry.cpp",
        "src/core/utils.cpp",
        "src/themes/extensions/post_insight_extension.cpp",
        "src/plugins/plugin_registry.cpp"
    )

    add_options(
        "feature_server",
        "plugin_tags",
        "plugin_archives",
        "plugin_rss",
        "plugin_search",
        "plugin_sitemap",
        "plugin_comments",
        "plugin_math",
        "plugin_authors",
        "plugin_forum",
        "plugin_cloud",
        "plugin_post_protect",
        "server_plugin_forum_api",
        "server_plugin_post_auth",
        "theme_aurora_runtime"
    )

    if has_config("feature_server") then
        add_files("src/core/http_server.cpp")
    end
    if has_config("plugin_tags") then add_files("src/plugins/tags/tag_plugin.cpp") end
    if has_config("plugin_archives") then add_files("src/plugins/archives/archive_plugin.cpp") end
    if has_config("plugin_rss") then add_files("src/plugins/rss/rss_plugin.cpp") end
    if has_config("plugin_search") then add_files("src/plugins/search/search_plugin.cpp") end
    if has_config("plugin_sitemap") then add_files("src/plugins/sitemap/sitemap_plugin.cpp") end
    if has_config("plugin_comments") then add_files("src/plugins/comments/comments_plugin.cpp") end
    if has_config("plugin_math") then add_files("src/plugins/math/math_plugin.cpp") end
    if has_config("plugin_authors") then add_files("src/plugins/authors/authors_plugin.cpp") end
    if has_config("plugin_cloud") then add_files("src/plugins/cloud/cloud_plugin.cpp") end
    if has_config("plugin_forum") then add_deps("plugin_forum_bundle") end
    if has_config("plugin_post_protect") then add_deps("plugin_post_protect_bundle") end
    if has_config("feature_server") and has_config("server_plugin_forum_api") then
        add_deps("server_plugin_forum_api_bundle")
    end
    if has_config("feature_server") and has_config("server_plugin_post_auth") then
        add_deps("server_plugin_post_auth_bundle")
    end
    if has_config("theme_aurora_runtime") then add_deps("theme_aurora_runtime_bundle") end

    add_defines("BLOGPP_BUILD_FEATURE_SERVER=" .. (has_config("feature_server") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_TAGS=" .. (has_config("plugin_tags") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_ARCHIVES=" .. (has_config("plugin_archives") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_RSS=" .. (has_config("plugin_rss") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_SEARCH=" .. (has_config("plugin_search") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_SITEMAP=" .. (has_config("plugin_sitemap") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_COMMENTS=" .. (has_config("plugin_comments") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_MATH=" .. (has_config("plugin_math") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_AUTHORS=" .. (has_config("plugin_authors") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_FORUM=" .. (has_config("plugin_forum") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_CLOUD=" .. (has_config("plugin_cloud") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_POST_PROTECT=" .. (has_config("plugin_post_protect") and "1" or "0"))
    add_defines("BLOGPP_BUILD_SERVER_PLUGIN_FORUM_API=" ..
        ((has_config("feature_server") and has_config("server_plugin_forum_api")) and "1" or "0"))
    add_defines("BLOGPP_BUILD_SERVER_PLUGIN_POST_AUTH=" ..
        ((has_config("feature_server") and has_config("server_plugin_post_auth")) and "1" or "0"))
    add_defines("BLOGPP_BUILD_THEME_AURORA_RUNTIME=" .. (has_config("theme_aurora_runtime") and "1" or "0"))

    if is_plat("windows") and has_config("feature_server") then
        add_links("ws2_32")
    end
