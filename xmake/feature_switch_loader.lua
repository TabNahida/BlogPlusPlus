local SECTION_ORDER = {"plugins", "server_plugins", "themes"}
local SECTION_PREFIX = {
    plugins = "plugin_",
    server_plugins = "server_plugin_",
    themes = "theme_"
}

local function sorted_keys(entries)
    local keys = {}
    for key, _ in pairs(entries or {}) do
        table.insert(keys, key)
    end
    table.sort(keys)
    return keys
end

local function option_name(section, key)
    local prefix = SECTION_PREFIX[section]
    if not prefix then
        raise("unknown feature section: " .. tostring(section))
    end
    return prefix .. key
end

local function is_enabled(section, key, entry)
    local enabled = has_config(option_name(section, key))
    if section == "server_plugins" then
        if entry.requires_server == nil or entry.requires_server then
            enabled = enabled and has_config("feature_server")
        end
    end
    return enabled
end

function blogpp_load_feature_switches()
    if type(blogpp_feature_switch_catalog) ~= "function" then
        raise("feature switch catalog is not defined, ensure xmake/feature_switches.lua is included")
    end
    local switches = blogpp_feature_switch_catalog()
    if type(switches) ~= "table" then
        raise("feature switch catalog must return a table")
    end
    for _, section in ipairs(SECTION_ORDER) do
        switches[section] = switches[section] or {}
    end
    return switches
end

function blogpp_register_feature_options(switches)
    for _, section in ipairs(SECTION_ORDER) do
        local entries = switches[section] or {}
        for _, key in ipairs(sorted_keys(entries)) do
            local entry = entries[key]
            option(option_name(section, key))
                set_default(entry.enabled ~= false)
                set_showmenu(true)
                set_description(entry.description or ("Enable " .. key))
            option_end()
        end
    end
end

function blogpp_collect_feature_option_names(switches)
    local names = {}
    for _, section in ipairs(SECTION_ORDER) do
        local entries = switches[section] or {}
        for _, key in ipairs(sorted_keys(entries)) do
            table.insert(names, option_name(section, key))
        end
    end
    return names
end

function blogpp_apply_feature_targets(switches)
    for _, section in ipairs(SECTION_ORDER) do
        local entries = switches[section] or {}
        for _, key in ipairs(sorted_keys(entries)) do
            local entry = entries[key]
            if is_enabled(section, key, entry) then
                if entry.kind == "core_file" then
                    add_files(entry.file)
                elseif entry.kind == "dep" then
                    add_deps(entry.dep)
                else
                    raise(("unknown feature kind for '%s.%s': %s")
                        :format(section, key, tostring(entry.kind)))
                end
            end
        end
    end
end

function blogpp_add_feature_defines(switches)
    for _, section in ipairs(SECTION_ORDER) do
        local entries = switches[section] or {}
        for _, key in ipairs(sorted_keys(entries)) do
            local entry = entries[key]
            local macro_name = entry.define
            if macro_name and macro_name ~= "" then
                local value = is_enabled(section, key, entry) and "1" or "0"
                add_defines(macro_name .. "=" .. value)
            end
        end
    end
end
