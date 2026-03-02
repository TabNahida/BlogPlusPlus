target("plugin_forum_bundle")
    set_kind("static")
    set_default(false)
    set_group("plugins")
    add_includedirs(path.join(os.projectdir(), "src"))
    if has_config("plugin_forum") then
        add_files(path.join(os.scriptdir(), "forum_plugin.cpp"))
    end
