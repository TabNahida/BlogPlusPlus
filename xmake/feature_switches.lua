function blogpp_feature_switch_catalog()
    return {
        plugins = {
        tags = {
            enabled = true,
            description = "Enable tags plugin",
            kind = "core_file",
            file = "src/plugins/tags/tag_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_TAGS"
        },
        archives = {
            enabled = true,
            description = "Enable archives plugin",
            kind = "core_file",
            file = "src/plugins/archives/archive_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_ARCHIVES"
        },
        rss = {
            enabled = true,
            description = "Enable RSS plugin",
            kind = "core_file",
            file = "src/plugins/rss/rss_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_RSS"
        },
        search = {
            enabled = true,
            description = "Enable search plugin",
            kind = "core_file",
            file = "src/plugins/search/search_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_SEARCH"
        },
        sitemap = {
            enabled = true,
            description = "Enable sitemap plugin",
            kind = "core_file",
            file = "src/plugins/sitemap/sitemap_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_SITEMAP"
        },
        comments = {
            enabled = true,
            description = "Enable comments plugin",
            kind = "core_file",
            file = "src/plugins/comments/comments_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_COMMENTS"
        },
        math = {
            enabled = true,
            description = "Enable markdown math rendering plugin",
            kind = "core_file",
            file = "src/plugins/math/math_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_MATH"
        },
        cloud = {
            enabled = true,
            description = "Enable cloud drive export plugin",
            kind = "core_file",
            file = "src/plugins/cloud/cloud_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_CLOUD"
        },
        authors = {
            enabled = true,
            description = "Enable author index plugin",
            kind = "core_file",
            file = "src/plugins/authors/authors_plugin.cpp",
            define = "BLOGPP_BUILD_PLUGIN_AUTHORS"
        },
        forum = {
            enabled = true,
            description = "Enable static forum page plugin",
            kind = "dep",
            dep = "plugin_forum_bundle",
            define = "BLOGPP_BUILD_PLUGIN_FORUM"
        },
        post_protect = {
            enabled = true,
            description = "Enable encrypted post static plugin",
            kind = "dep",
            dep = "plugin_post_protect_bundle",
            define = "BLOGPP_BUILD_PLUGIN_POST_PROTECT"
        }
        },
        server_plugins = {
        forum_api = {
            enabled = true,
            description = "Enable server-side forum API plugin",
            kind = "dep",
            dep = "server_plugin_forum_api_bundle",
            define = "BLOGPP_BUILD_SERVER_PLUGIN_FORUM_API",
            requires_server = true
        },
        post_auth = {
            enabled = true,
            description = "Enable server-side post unlock API",
            kind = "dep",
            dep = "server_plugin_post_auth_bundle",
            define = "BLOGPP_BUILD_SERVER_PLUGIN_POST_AUTH",
            requires_server = true
        }
        },
        themes = {
        aurora_runtime = {
            enabled = true,
            description = "Enable aurora theme runtime extension",
            kind = "dep",
            dep = "theme_aurora_runtime_bundle",
            define = "BLOGPP_BUILD_THEME_AURORA_RUNTIME"
        }
        }
    }
end
