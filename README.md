# BlogPlusPlus

A modern C++ blog framework (XMake-based) with:

- Static generation (`blogpp build`)
- Optional local server runtime (`blogpp serve`)
- Build-time feature toggles for plugins
- Static plugins + server plugins
- Multi-theme rendering with palette switching and background image support
- C++ template runtime extension points (EJS-like logic via standalone C++ files)

## Features

- Content model:
  - Posts: `content/posts/*.md`
  - Pages: `content/pages/*.md`
- Core pages:
  - Homepage, post pages, page pages, `404`
- Runtime:
  - Built-in HTTP static server
  - Dynamic API extension points (server plugins)
- Performance:
  - Markdown regex cache optimization
  - Static file cache in server mode (`server_cache_mb`)

## Built-In Plugins

Static plugins (build output):

- `tags` -> `/tags/` and tag detail pages
- `authors` -> `/authors/` and author detail pages (multi-author support)
- `archives` -> `/archives/`
- `rss` -> `/feed.xml`
- `search` -> `/search/` + `/search-index.json`
- `sitemap` -> `/sitemap.xml`
- `math` -> KaTeX auto-render injection for markdown formulas
- `comments` -> Post page comment system injection (Giscus/Disqus)
- `forum` -> `/forum/` + `/forum-data.json` (static read-only fallback)
- `post_protect` -> encrypted post pages (password unlock UI)
- `cloud` -> `/cloud/` + `/cloud-sync-manifest.json`

Server plugins (serve mode):

- `forum_api` -> `/api/forum/threads`, `/api/forum/posts`, `/api/forum/users` (+ legacy `/api/forum/messages`)
- `post_auth` -> `/api/post/unlock` for protected article unlock in server mode (`route` + `password`, form-urlencoded)

## Themes

- `aurora` (glass + neon)
- `paperwave` (editorial paper)
- `neonpulse` (futuristic grid)
- `summit` (clean magazine)

Theme capabilities:

- Background image (`background_image`)
- Runtime palette switching (`theme_palettes`)
- Animations + reduced-motion fallback

## Build

```powershell
xmake f -m release
xmake
```

Hexo-like switch configuration layout:

- Switch catalog (plugins/server plugins/themes):
  - `xmake/feature_switches.lua`
- Switch processor/loader:
  - `xmake/feature_switch_loader.lua`
- Main build entry:
  - `xmake.lua` (loads switch catalog + applies it)

Build-time feature examples:

```powershell
# Disable server mode in binary
xmake f --feature_server=n

# Disable cloud and forum static plugins in binary
xmake f --plugin_cloud=n --plugin_forum=n

# Disable forum API server plugin in binary
xmake f --server_plugin_forum_api=n

# Disable encrypted article plugin and unlock API
xmake f --plugin_post_protect=n --server_plugin_post_auth=n
```

## Build File Layout

- Main build entry: `xmake.lua` (core framework build remains here)
- Plugin source layout: `src/plugins/<plugin_name>/...`
- Feature toggles are centralized (Hexo-style) and processed by loader:
  - `xmake/feature_switches.lua`
  - `xmake/feature_switch_loader.lua`
- Multi-level xmake files are regular target-based project files and auto-discovered recursively:
  - `src/plugins/forum/xmake.lua` -> `plugin_forum_bundle`
  - `src/plugins/forum_api/xmake.lua` -> `server_plugin_forum_api_bundle`
  - `src/plugins/post_protect/xmake.lua` -> `plugin_post_protect_bundle`
  - `src/plugins/post_auth_api/xmake.lua` -> `server_plugin_post_auth_bundle`
  - `themes/aurora/xmake.lua` -> `theme_aurora_runtime_bundle`

CI compatibility validation:

- Workflow file: `.github/workflows/compatibility-ci.yml`
- Trigger: `push`, `pull_request`, `workflow_dispatch`
- Coverage:
  - OS/arch matrix: Ubuntu x86_64, Windows x64, macOS x86_64, macOS arm64
  - Build profiles: full features + minimal features
  - API smoke: `/api/forum/users` + `/api/post/unlock`

## C++ Template Runtime Extensions

Template files stay pure HTML placeholders. Dynamic logic is implemented in standalone C++ files and registered through extension points.

- Registry API:
  - `src/core/theme_extension_registry.h`
  - `src/core/theme_extension_registry.cpp`
- Shared extension example:
  - `src/themes/extensions/post_insight_extension.cpp`
- Theme-specific extension examples:
  - `themes/aurora/runtime.cpp`
  - `themes/aurora/xmake.lua`

Each runtime extension is compiled by xmake and applied at render time (build/serve), similar to EJS capabilities but with typed C++ code.

## Usage

```powershell
# Bootstrap config + content skeleton
xmake run blogpp init

# Static export
xmake run blogpp build

# Build + local server (default 4000)
xmake run blogpp serve

# Build + local server on custom port
xmake run blogpp serve 4010

# Create a new post
xmake run blogpp new "My New Post"
```

## Config

`blogpp.yml` (example):

```yaml
title: BlogPlusPlus
subtitle: A modern C++ blog framework with static export and built-in server mode.
url: http://localhost:4000
theme: aurora
public_dir: public
plugins:
  - tags
  - authors
  - archives
  - rss
  - search
  - sitemap
  - math
  - comments
  - forum
  - cloud
  - post_protect
server_plugins:
  - forum_api
  - post_auth
menu:
  Home: /
  Authors: /authors/
  Archives: /archives/
  Tags: /tags/
  Search: /search/
  Forum: /forum/
  Cloud: /cloud/
social:
  GitHub: https://github.com/
  RSS: /feed.xml
theme_palettes:
  - ocean
  - sunset
  - forest
background_image:
server_cache_mb: 32
comments_provider: giscus
comments_giscus_repo:
comments_giscus_repo_id:
comments_giscus_category: Announcements
comments_giscus_category_id:
comments_disqus_shortname:
cloud_provider: s3
cloud_bucket:
cloud_public_prefix:
```

Config compatibility aliases (Hexo-style):

- `url` -> `base_url`
- `public_dir` -> `output`
- `subtitle`/`tagline` -> `description`
- `menu` -> `nav`
- `theme_palettes`/`palettes`/`palette`

Supported config formats:

- Inline CSV: `plugins: tags, archives, rss`
- YAML-like list:
  - `plugins:`
  - `  - tags`
  - `  - archives`
- YAML-like map:
  - `menu:`
  - `  Home: /`
  - `  Archives: /archives/`

## Hexo Gap & Roadmap

Already improved in this iteration:

- More flexible config parser (list/map/inline/comment handling)
- Hexo-friendly key aliases (`url`, `public_dir`, `menu`, `subtitle`)
- `menu.*` and `social.*` map parsing for nav/footer links
- Config file fallback search: `blogpp.yml`, `_config.blogpp.yml`, `_config.yml`
- `blogpp init` one-command project bootstrap

Still missing vs mainstream Hexo themes (next priorities):

- Category taxonomy and category pages
- Pagination (`per_page`) with index/archive page navigation
- Post TOC generation and heading anchor links
- Post `cover`/`banner` pipeline and image optimization
- i18n and date/time localization
- Theme-level config inheritance (`themes/<theme>/_config.yml`)

## Markdown Front Matter

```markdown
---
title: Hello Blog
date: 2026-03-01
tags: c++, blog, demo
summary: A short summary for list cards and feed.
---

# Heading
Math: $E = mc^2$
```

## Output Structure

After `build`, files are generated under `public/`:

- `/index.html`
- `/posts/<slug>/index.html`
- `/about/index.html`
- `/tags/`, `/archives/`, `/search/`, `/forum/`, `/cloud/`
- `/feed.xml`, `/sitemap.xml`, `/search-index.json`, `/cloud-sync-manifest.json`, `/forum-data.json`
