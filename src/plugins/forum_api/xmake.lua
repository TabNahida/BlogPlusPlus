target("server_plugin_forum_api_bundle")
    set_kind("static")
    set_default(false)
    set_group("plugins/server")
    add_includedirs(path.join(os.projectdir(), "src"))
    if has_config("feature_server") and has_config("server_plugin_forum_api") then
        add_files(path.join(os.scriptdir(), "forum_api_plugin.cpp"))
    end
