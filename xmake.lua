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

option("plugin_forum")
    set_default(true)
    set_showmenu(true)
    set_description("Enable static forum page plugin")
option_end()

option("plugin_cloud")
    set_default(true)
    set_showmenu(true)
    set_description("Enable cloud drive export plugin")
option_end()

option("server_plugin_forum_api")
    set_default(true)
    set_showmenu(true)
    set_description("Enable server-side forum API plugin")
option_end()

includes("themes/aurora/xmake.lua")
blogpp_define_theme_aurora_options()

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
        "plugin_forum",
        "plugin_cloud",
        "server_plugin_forum_api"
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
    if has_config("plugin_forum") then add_files("src/plugins/forum/forum_plugin.cpp") end
    if has_config("plugin_cloud") then add_files("src/plugins/cloud/cloud_plugin.cpp") end
    if has_config("feature_server") and has_config("server_plugin_forum_api") then
        add_files("src/plugins/forum_api/forum_api_plugin.cpp")
    end

    add_defines("BLOGPP_BUILD_FEATURE_SERVER=" .. (has_config("feature_server") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_TAGS=" .. (has_config("plugin_tags") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_ARCHIVES=" .. (has_config("plugin_archives") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_RSS=" .. (has_config("plugin_rss") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_SEARCH=" .. (has_config("plugin_search") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_SITEMAP=" .. (has_config("plugin_sitemap") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_COMMENTS=" .. (has_config("plugin_comments") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_MATH=" .. (has_config("plugin_math") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_FORUM=" .. (has_config("plugin_forum") and "1" or "0"))
    add_defines("BLOGPP_BUILD_PLUGIN_CLOUD=" .. (has_config("plugin_cloud") and "1" or "0"))
    add_defines("BLOGPP_BUILD_SERVER_PLUGIN_FORUM_API=" ..
        ((has_config("feature_server") and has_config("server_plugin_forum_api")) and "1" or "0"))

    blogpp_apply_theme_aurora_build()

    if is_plat("windows") and has_config("feature_server") then
        add_links("ws2_32")
    end
