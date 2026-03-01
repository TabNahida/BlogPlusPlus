#include "plugins/plugin_registry.h"

#include "core/build_flags.h"

#if BLOGPP_BUILD_PLUGIN_ARCHIVES
#include "plugins/archives/archive_plugin.h"
#endif
#if BLOGPP_BUILD_PLUGIN_RSS
#include "plugins/rss/rss_plugin.h"
#endif
#if BLOGPP_BUILD_PLUGIN_SEARCH
#include "plugins/search/search_plugin.h"
#endif
#if BLOGPP_BUILD_PLUGIN_SITEMAP
#include "plugins/sitemap/sitemap_plugin.h"
#endif
#if BLOGPP_BUILD_PLUGIN_TAGS
#include "plugins/tags/tag_plugin.h"
#endif
#if BLOGPP_BUILD_PLUGIN_COMMENTS
#include "plugins/comments/comments_plugin.h"
#endif
#if BLOGPP_BUILD_PLUGIN_MATH
#include "plugins/math/math_plugin.h"
#endif
#if BLOGPP_BUILD_PLUGIN_FORUM
#include "plugins/forum/forum_plugin.h"
#endif
#if BLOGPP_BUILD_PLUGIN_CLOUD
#include "plugins/cloud/cloud_plugin.h"
#endif
#if BLOGPP_BUILD_SERVER_PLUGIN_FORUM_API
#include "plugins/forum_api/forum_api_plugin.h"
#endif

namespace blogpp {

std::vector<std::unique_ptr<Plugin>> create_static_plugins() {
    std::vector<std::unique_ptr<Plugin>> plugins;
#if BLOGPP_BUILD_PLUGIN_TAGS
    plugins.emplace_back(std::make_unique<TagPlugin>());
#endif
#if BLOGPP_BUILD_PLUGIN_ARCHIVES
    plugins.emplace_back(std::make_unique<ArchivePlugin>());
#endif
#if BLOGPP_BUILD_PLUGIN_RSS
    plugins.emplace_back(std::make_unique<RssPlugin>());
#endif
#if BLOGPP_BUILD_PLUGIN_SEARCH
    plugins.emplace_back(std::make_unique<SearchPlugin>());
#endif
#if BLOGPP_BUILD_PLUGIN_SITEMAP
    plugins.emplace_back(std::make_unique<SitemapPlugin>());
#endif
#if BLOGPP_BUILD_PLUGIN_FORUM
    plugins.emplace_back(std::make_unique<ForumPlugin>());
#endif
#if BLOGPP_BUILD_PLUGIN_CLOUD
    plugins.emplace_back(std::make_unique<CloudPlugin>());
#endif
#if BLOGPP_BUILD_PLUGIN_MATH
    plugins.emplace_back(std::make_unique<MathPlugin>());
#endif
#if BLOGPP_BUILD_PLUGIN_COMMENTS
    plugins.emplace_back(std::make_unique<CommentsPlugin>());
#endif
    return plugins;
}

std::vector<std::unique_ptr<ServerPlugin>> create_server_plugins() {
    std::vector<std::unique_ptr<ServerPlugin>> plugins;
#if BLOGPP_BUILD_SERVER_PLUGIN_FORUM_API
    plugins.emplace_back(std::make_unique<ForumApiPlugin>());
#endif
    return plugins;
}

}  // namespace blogpp
