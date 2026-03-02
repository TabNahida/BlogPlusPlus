target("theme_aurora_runtime_bundle")
    set_kind("static")
    set_default(false)
    set_group("themes/runtime")
    add_includedirs(path.join(os.projectdir(), "src"))
    if has_config("theme_aurora_runtime") then
        add_files(path.join(os.scriptdir(), "runtime.cpp"))
    end
