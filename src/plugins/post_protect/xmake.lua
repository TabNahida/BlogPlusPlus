target("plugin_post_protect_bundle")
    set_kind("static")
    set_default(false)
    set_group("plugins")
    add_includedirs(path.join(os.projectdir(), "src"))
    if has_config("plugin_post_protect") then
        add_files(path.join(os.scriptdir(), "post_protect_plugin.cpp"))
    end
