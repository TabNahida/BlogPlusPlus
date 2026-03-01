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
- `archives` -> `/archives/`
- `rss` -> `/feed.xml`
- `search` -> `/search/` + `/search-index.json`
- `sitemap` -> `/sitemap.xml`
- `math` -> KaTeX auto-render injection for markdown formulas
- `comments` -> Post page comment system injection (Giscus/Disqus)
- `forum` -> `/forum/` page with frontend client
- `cloud` -> `/cloud/` + `/cloud-sync-manifest.json`

Server plugins (serve mode):

- `forum_api` -> `/api/forum/messages` (`GET`, `POST`)

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

Build-time feature examples:

```powershell
# Disable server mode in binary
xmake f --feature_server=n

# Disable cloud and forum static plugins in binary
xmake f --plugin_cloud=n --plugin_forum=n

# Disable forum API server plugin in binary
xmake f --server_plugin_forum_api=n
```

## Build File Layout

- Main build entry: `xmake.lua` (core framework + common plugin build options)
- Plugin source layout: `src/plugins/<plugin_name>/...`
- Optional plugin-specific xmake:
  - only add `src/plugins/<plugin_name>/xmake.lua` when the plugin needs special build config
- Theme runtime xmake:
  - only add `themes/<theme>/xmake.lua` for themes that need runtime C++ extension compilation

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
description: A modern C++ blog framework with static export and built-in server mode.
base_url: http://localhost:4000
theme: aurora
output: public
plugins: tags, archives, rss, search, sitemap, math, comments, forum, cloud
server_plugins: forum_api
nav: Home:/, Archives:/archives/, Tags:/tags/, Search:/search/, Forum:/forum/, Cloud:/cloud/
theme_palettes: ocean, sunset, forest
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
- `/feed.xml`, `/sitemap.xml`, `/search-index.json`, `/cloud-sync-manifest.json`
