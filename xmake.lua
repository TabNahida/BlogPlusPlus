set_project("BlogPlusPlus")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")
set_languages("cxx20")

includes("xmake/feature_switches.lua")
includes("xmake/feature_switch_loader.lua")

local feature_switches = blogpp_load_feature_switches()
local feature_option_names = blogpp_collect_feature_option_names(feature_switches)

option("feature_server")
    set_default(true)
    set_showmenu(true)
    set_description("Enable builtin HTTP server mode")
option_end()

blogpp_register_feature_options(feature_switches)

includes("src/plugins/**/xmake.lua")
includes("themes/**/xmake.lua")

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
        table.unpack(feature_option_names)
    )

    if has_config("feature_server") then
        add_files("src/core/http_server.cpp")
    end
    blogpp_apply_feature_targets(feature_switches)

    add_defines("BLOGPP_BUILD_FEATURE_SERVER=" .. (has_config("feature_server") and "1" or "0"))
    blogpp_add_feature_defines(feature_switches)

    if is_plat("windows") and has_config("feature_server") then
        add_links("ws2_32")
    end
