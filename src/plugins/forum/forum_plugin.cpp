#include "plugins/forum/forum_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <algorithm>
#include <set>
#include <sstream>

namespace blogpp {

namespace {

struct ForumThreadSeed {
    int id = 0;
    std::string title;
    std::string author;
    std::string created_at;
};

struct ForumPostSeed {
    int id = 0;
    int thread_id = 0;
    std::string author;
    std::string text;
    std::string created_at;
};

std::string threads_to_json(const std::vector<ForumThreadSeed>& threads) {
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < threads.size(); ++i) {
        const auto& thread = threads[i];
        out << "  {\n"
            << "    \"id\": " << thread.id << ",\n"
            << "    \"title\": \"" << escape_json(thread.title) << "\",\n"
            << "    \"author\": \"" << escape_json(thread.author) << "\",\n"
            << "    \"created_at\": \"" << escape_json(thread.created_at) << "\"\n"
            << "  }";
        if (i + 1 < threads.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]";
    return out.str();
}

std::string posts_to_json(const std::vector<ForumPostSeed>& posts) {
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < posts.size(); ++i) {
        const auto& post = posts[i];
        out << "  {\n"
            << "    \"id\": " << post.id << ",\n"
            << "    \"thread_id\": " << post.thread_id << ",\n"
            << "    \"author\": \"" << escape_json(post.author) << "\",\n"
            << "    \"text\": \"" << escape_json(post.text) << "\",\n"
            << "    \"created_at\": \"" << escape_json(post.created_at) << "\"\n"
            << "  }";
        if (i + 1 < posts.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]";
    return out.str();
}

std::string users_to_json(const std::set<std::string>& users) {
    std::ostringstream out;
    out << "[";
    size_t index = 0;
    for (const auto& user : users) {
        if (index++ > 0) {
            out << ",";
        }
        out << "\"" << escape_json(user) << "\"";
    }
    out << "]";
    return out.str();
}

}  // namespace

std::string ForumPlugin::name() const {
    return "forum";
}

void ForumPlugin::apply(SiteBuilder& builder) {
    const auto& site = builder.context();

    std::vector<ForumThreadSeed> threads;
    std::vector<ForumPostSeed> posts;
    std::set<std::string> users;

    int next_thread_id = 1;
    int next_post_id = 1;
    const auto now = current_datetime();

    threads.push_back({next_thread_id++, "Welcome to BlogPlusPlus Forum", "BlogPlusPlus", now});
    posts.push_back({next_post_id++, 1, "BlogPlusPlus", "In static mode this page is read-only.", now});
    users.insert("BlogPlusPlus");

    const size_t seed_limit = std::min<size_t>(site.posts.size(), 8);
    for (size_t i = 0; i < seed_limit; ++i) {
        const auto& post = site.posts[i];
        const auto author =
            !post.authors.empty() && !trim(post.authors.front()).empty() ? post.authors.front() : "Author";
        const auto thread_id = next_thread_id++;
        threads.push_back({thread_id, post.title, author, post.date});
        posts.push_back({next_post_id++, thread_id, author, post.summary, post.date});
        users.insert(author);
    }

    std::ostringstream seed_json;
    seed_json << "{\n"
              << "  \"threads\": " << threads_to_json(threads) << ",\n"
              << "  \"posts\": " << posts_to_json(posts) << ",\n"
              << "  \"users\": " << users_to_json(users) << "\n"
              << "}\n";
    builder.add_generated("/forum-data.json", seed_json.str(), "application/json");

    const std::string body =
        "<section class=\"forum-page\">"
        "<p class=\"kicker\">Community</p>"
        "<h1>Forum Hub</h1>"
        "<p class=\"desc\">Server mode supports multi-thread and multi-user posting. Static mode falls back to read-only data.</p>"
        "<div class=\"forum-layout\">"
        "<aside class=\"forum-sidebar\">"
        "<h3>Threads</h3>"
        "<ul id=\"forum-threads\" class=\"post-list\"></ul>"
        "<form id=\"forum-thread-form\" class=\"forum-form\">"
        "<input id=\"forum-thread-author\" name=\"author\" placeholder=\"Your name\" maxlength=\"40\" />"
        "<input id=\"forum-thread-title\" name=\"title\" placeholder=\"New thread title\" maxlength=\"120\" />"
        "<textarea id=\"forum-thread-text\" name=\"text\" rows=\"3\" maxlength=\"360\" placeholder=\"First post (optional)\"></textarea>"
        "<button type=\"submit\">Create Thread</button>"
        "</form>"
        "</aside>"
        "<div class=\"forum-main\">"
        "<h3 id=\"forum-thread-title-view\">Posts</h3>"
        "<ul id=\"forum-posts\" class=\"post-list\"></ul>"
        "<form id=\"forum-post-form\" class=\"forum-form\">"
        "<input id=\"forum-post-author\" name=\"author\" placeholder=\"Your name\" maxlength=\"40\" />"
        "<textarea id=\"forum-post-text\" name=\"text\" rows=\"4\" maxlength=\"360\" placeholder=\"Reply in thread...\"></textarea>"
        "<button type=\"submit\">Reply</button>"
        "</form>"
        "<p id=\"forum-mode\" class=\"meta\"></p>"
        "</div>"
        "</div>"
        "<script>"
        "const threadList=document.getElementById('forum-threads');"
        "const postList=document.getElementById('forum-posts');"
        "const threadForm=document.getElementById('forum-thread-form');"
        "const postForm=document.getElementById('forum-post-form');"
        "const titleView=document.getElementById('forum-thread-title-view');"
        "const modeView=document.getElementById('forum-mode');"
        "let mode='api';"
        "let staticData={threads:[],posts:[],users:[]};"
        "let threads=[];"
        "let activeThreadId=0;"
        "function esc(v){"
        "const s=String(v||'');"
        "return s.replaceAll('&','&amp;').replaceAll('<','&lt;').replaceAll('>','&gt;').replaceAll('\"','&quot;').replaceAll(\"'\",'&#39;');"
        "}"
        "function renderThreads(){"
        "threadList.innerHTML=threads.map(t=>`<li><a href=\\\"#\\\" data-thread-id=\\\"${t.id}\\\">${esc(t.title)}</a><span class=\\\"meta\\\">${esc(t.author)}</span></li>`).join('');"
        "threadList.querySelectorAll('a[data-thread-id]').forEach(el=>el.addEventListener('click',e=>{e.preventDefault();selectThread(Number(el.dataset.threadId||0));}));"
        "}"
        "function renderPosts(items){"
        "postList.innerHTML=(items||[]).map(p=>`<li><strong>${esc(p.author)}</strong><span class=\\\"meta\\\">${esc(p.created_at)}</span><p>${esc(p.text)}</p></li>`).join('');"
        "}"
        "async function fetchThreads(){"
        "try{"
        "const res=await fetch('/api/forum/threads');"
        "if(!res.ok){throw new Error('api');}"
        "threads=await res.json();"
        "mode='api';"
        "}catch(_){"
        "const fallback=await fetch('/forum-data.json').then(r=>r.json());"
        "staticData=fallback||{threads:[],posts:[],users:[]};"
        "threads=staticData.threads||[];"
        "mode='static';"
        "}"
        "modeView.textContent=mode==='api'?'Mode: live server API':'Mode: static fallback (read-only)';"
        "renderThreads();"
        "if(threads.length){selectThread(Number(threads[0].id||0));}"
        "toggleForms();"
        "}"
        "function toggleForms(){"
        "const disabled=mode!=='api';"
        "threadForm.querySelectorAll('input,textarea,button').forEach(el=>el.disabled=disabled);"
        "postForm.querySelectorAll('input,textarea,button').forEach(el=>el.disabled=disabled);"
        "}"
        "async function selectThread(threadId){"
        "activeThreadId=threadId;"
        "const thread=threads.find(t=>Number(t.id)===threadId);"
        "titleView.textContent=thread?`Posts: ${thread.title}`:'Posts';"
        "if(mode==='api'){"
        "const res=await fetch('/api/forum/posts?thread_id='+encodeURIComponent(String(threadId)));"
        "if(!res.ok){renderPosts([]);return;}"
        "renderPosts(await res.json());"
        "}else{"
        "renderPosts((staticData.posts||[]).filter(p=>Number(p.thread_id)===threadId));"
        "}"
        "}"
        "threadForm.addEventListener('submit',async(e)=>{"
        "e.preventDefault();"
        "if(mode!=='api'){return;}"
        "const author=(document.getElementById('forum-thread-author').value||'').trim()||'Anonymous';"
        "const title=(document.getElementById('forum-thread-title').value||'').trim();"
        "const text=(document.getElementById('forum-thread-text').value||'').trim();"
        "if(!title){return;}"
        "const body=new URLSearchParams({author,title,text}).toString();"
        "const res=await fetch('/api/forum/threads',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});"
        "if(!res.ok){return;}"
        "document.getElementById('forum-thread-title').value='';"
        "document.getElementById('forum-thread-text').value='';"
        "await fetchThreads();"
        "});"
        "postForm.addEventListener('submit',async(e)=>{"
        "e.preventDefault();"
        "if(mode!=='api'||!activeThreadId){return;}"
        "const author=(document.getElementById('forum-post-author').value||'').trim()||'Anonymous';"
        "const text=(document.getElementById('forum-post-text').value||'').trim();"
        "if(!text){return;}"
        "const body=new URLSearchParams({author,text,thread_id:String(activeThreadId)}).toString();"
        "const res=await fetch('/api/forum/posts',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});"
        "if(!res.ok){return;}"
        "document.getElementById('forum-post-text').value='';"
        "await selectThread(activeThreadId);"
        "});"
        "fetchThreads();"
        "</script>"
        "</section>";

    builder.add_generated("/forum/", builder.render_layout_page("Forum", body, "Community forum"), "text/html");
}

}  // namespace blogpp
