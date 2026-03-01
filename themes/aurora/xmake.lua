function blogpp_define_theme_aurora_options()
    option("theme_aurora_runtime")
        set_default(true)
        set_showmenu(true)
        set_description("Enable aurora theme runtime extension")
    option_end()
end

function blogpp_apply_theme_aurora_build()
    add_options("theme_aurora_runtime")
    add_defines("BLOGPP_BUILD_THEME_AURORA_RUNTIME=" ..
        (has_config("theme_aurora_runtime") and "1" or "0"))
    if has_config("theme_aurora_runtime") then
        add_files("themes/aurora/runtime.cpp")
    end
end
