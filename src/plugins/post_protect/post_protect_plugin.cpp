#include "plugins/post_protect/post_protect_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>

namespace blogpp {

namespace {

bool parse_bool(const std::string& raw) {
    const auto value = to_lower(trim(raw));
    return value == "1" || value == "true" || value == "yes" || value == "on";
}

bool find_post_content_section(const std::string& html,
                               size_t& section_begin,
                               size_t& content_begin,
                               size_t& content_end,
                               size_t& section_end) {
    const auto section_start = html.find("<section class=\"post-content\"");
    if (section_start == std::string::npos) {
        return false;
    }
    const auto open_end = html.find('>', section_start);
    if (open_end == std::string::npos) {
        return false;
    }
    section_begin = section_start;
    content_begin = open_end + 1;
    const auto close_pos = html.find("</section>", content_begin);
    if (close_pos == std::string::npos) {
        return false;
    }
    content_end = close_pos;
    section_end = close_pos + std::string("</section>").size();
    return true;
}

std::string xor_cipher(const std::string& input, const std::string& password) {
    if (password.empty()) {
        return input;
    }
    std::string out = input;
    for (size_t i = 0; i < out.size(); ++i) {
        out[i] = static_cast<char>(static_cast<unsigned char>(out[i]) ^
                                   static_cast<unsigned char>(password[i % password.size()]));
    }
    return out;
}

std::string base64_encode(const std::string& input) {
    static const char* kTable =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    out.reserve(((input.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i + 3 <= input.size()) {
        const uint32_t value =
            (static_cast<uint32_t>(static_cast<unsigned char>(input[i])) << 16) |
            (static_cast<uint32_t>(static_cast<unsigned char>(input[i + 1])) << 8) |
            static_cast<uint32_t>(static_cast<unsigned char>(input[i + 2]));
        out.push_back(kTable[(value >> 18) & 0x3F]);
        out.push_back(kTable[(value >> 12) & 0x3F]);
        out.push_back(kTable[(value >> 6) & 0x3F]);
        out.push_back(kTable[value & 0x3F]);
        i += 3;
    }

    const size_t remain = input.size() - i;
    if (remain == 1) {
        const uint32_t value = static_cast<uint32_t>(static_cast<unsigned char>(input[i])) << 16;
        out.push_back(kTable[(value >> 18) & 0x3F]);
        out.push_back(kTable[(value >> 12) & 0x3F]);
        out.push_back('=');
        out.push_back('=');
    } else if (remain == 2) {
        const uint32_t value =
            (static_cast<uint32_t>(static_cast<unsigned char>(input[i])) << 16) |
            (static_cast<uint32_t>(static_cast<unsigned char>(input[i + 1])) << 8);
        out.push_back(kTable[(value >> 18) & 0x3F]);
        out.push_back(kTable[(value >> 12) & 0x3F]);
        out.push_back(kTable[(value >> 6) & 0x3F]);
        out.push_back('=');
    }
    return out;
}

}  // namespace

std::string PostProtectPlugin::name() const {
    return "post_protect";
}

void PostProtectPlugin::apply(SiteBuilder& builder) {
    auto& site = builder.context();

    for (const auto& post : site.posts) {
        const auto meta_password = post.meta.find("password");
        const auto meta_encrypted = post.meta.find("encrypted");
        const bool should_encrypt = (meta_password != post.meta.end() && !trim(meta_password->second).empty()) ||
                                    (meta_encrypted != post.meta.end() && parse_bool(meta_encrypted->second));
        if (!should_encrypt) {
            continue;
        }
        if (meta_password == post.meta.end() || trim(meta_password->second).empty()) {
            continue;
        }

        auto file_it = std::find_if(site.generated.begin(), site.generated.end(), [&](const OutputFile& file) {
            return file.route == post.route && file.mime_type == "text/html";
        });
        if (file_it == site.generated.end()) {
            continue;
        }

        size_t section_begin = 0;
        size_t content_begin = 0;
        size_t content_end = 0;
        size_t section_end = 0;
        if (!find_post_content_section(file_it->content, section_begin, content_begin, content_end, section_end)) {
            continue;
        }

        const auto plaintext = file_it->content.substr(content_begin, content_end - content_begin);
        const auto marker_payload = "BLOGPP_POST_PROTECT::" + plaintext;
        const auto encrypted = xor_cipher(marker_payload, trim(meta_password->second));
        const auto payload_b64 = base64_encode(encrypted);

        std::ostringstream replacement;
        replacement
            << "<section class=\"post-content post-protect\" data-route=\"" << html_escape(post.route) << "\">"
            << "<div class=\"post-protect-panel\">"
            << "<h3>Protected Post</h3>"
            << "<p class=\"meta\">This post is encrypted. Enter password to unlock.</p>"
            << "<input class=\"post-protect-input\" type=\"password\" placeholder=\"Password\" />"
            << "<button class=\"post-protect-btn\" type=\"button\">Unlock</button>"
            << "<p class=\"meta post-protect-status\"></p>"
            << "<div class=\"post-protect-target\"></div>"
            << "</div>"
            << "<script>"
            << "(function(){"
            << "const host=document.currentScript&&document.currentScript.closest('.post-protect');"
            << "if(!host){return;}"
            << "const input=host.querySelector('.post-protect-input');"
            << "const button=host.querySelector('.post-protect-btn');"
            << "const status=host.querySelector('.post-protect-status');"
            << "const target=host.querySelector('.post-protect-target');"
            << "const route=host.getAttribute('data-route')||window.location.pathname;"
            << "const payload='" << escape_json(payload_b64) << "';"
            << "function xorDecode(binary,password){"
            << "if(!password){return '';}"
            << "let out='';"
            << "for(let i=0;i<binary.length;i++){"
            << "const code=binary.charCodeAt(i)^password.charCodeAt(i%password.length);"
            << "out+=String.fromCharCode(code);"
            << "}"
            << "return out;"
            << "}"
            << "function decodeLocal(password){"
            << "try{"
            << "const bin=atob(payload);"
            << "const decoded=xorDecode(bin,password);"
            << "if(!decoded.startsWith('BLOGPP_POST_PROTECT::')){return '';}"
            << "return decoded.slice('BLOGPP_POST_PROTECT::'.length);"
            << "}catch(_){return '';}"
            << "}"
            << "async function decodeRemote(password){"
            << "const body=new URLSearchParams({route,password}).toString();"
            << "const res=await fetch('/api/post/unlock',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});"
            << "if(!res.ok){throw new Error('unlock-failed');}"
            << "const data=await res.json();"
            << "if(!data||!data.html){throw new Error('invalid-data');}"
            << "return data.html;"
            << "}"
            << "button.addEventListener('click',async function(){"
            << "const password=(input.value||'').trim();"
            << "if(!password){status.textContent='Password is required.';return;}"
            << "let html='';"
            << "try{html=await decodeRemote(password);}catch(_){html=decodeLocal(password);}"
            << "if(!html){status.textContent='Wrong password.';return;}"
            << "target.innerHTML=html;"
            << "status.textContent='Unlocked.';"
            << "host.classList.add('unlocked');"
            << "});"
            << "})();"
            << "</script>"
            << "</section>";

        file_it->content.replace(section_begin, section_end - section_begin, replacement.str());
    }
}

}  // namespace blogpp
