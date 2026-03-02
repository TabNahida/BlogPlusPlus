#include "plugins/post_auth_api/post_auth_api_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <algorithm>
#include <string>
#include <unordered_map>

namespace blogpp {

namespace {

std::unordered_map<std::string, std::string> parse_form_urlencoded(const std::string& body) {
    std::unordered_map<std::string, std::string> values;
    for (const auto& part : split(body, '&')) {
        if (part.empty()) {
            continue;
        }
        const auto pos = part.find('=');
        const auto key = url_decode(pos == std::string::npos ? part : part.substr(0, pos));
        const auto value = url_decode(pos == std::string::npos ? "" : part.substr(pos + 1));
        if (!key.empty()) {
            values[key] = value;
        }
    }
    return values;
}

std::string normalize_route(std::string route) {
    route = trim(route);
    if (route.empty()) {
        return "/";
    }
    if (route.front() != '/') {
        route = "/" + route;
    }
    if (!route.empty() && route.back() != '/') {
        route.push_back('/');
    }
    return route;
}

}  // namespace

std::string PostAuthApiPlugin::name() const {
    return "post_auth";
}

bool PostAuthApiPlugin::handle(const HttpRequest& request, HttpResponse& response, SiteBuilder& builder) {
    if (request.path != "/api/post/unlock") {
        return false;
    }

    response.content_type = "application/json; charset=utf-8";
    const auto method = to_lower(request.method);
    if (method != "post") {
        response.status_code = 405;
        response.status_text = "Method Not Allowed";
        response.body = "{\"error\":\"method not allowed\"}";
        return true;
    }

    const auto form = parse_form_urlencoded(request.body);
    auto route = normalize_route(form.count("route") ? form.at("route") : "");
    auto password = trim(form.count("password") ? form.at("password") : "");
    if (route.empty() || password.empty()) {
        response.status_code = 400;
        response.status_text = "Bad Request";
        response.body = "{\"error\":\"route and password are required\"}";
        return true;
    }

    const auto& posts = builder.context().posts;
    const auto post_it = std::find_if(posts.begin(), posts.end(), [&](const ContentItem& post) {
        return post.route == route;
    });
    if (post_it == posts.end()) {
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = "{\"error\":\"post not found\"}";
        return true;
    }

    const auto pw_it = post_it->meta.find("password");
    if (pw_it == post_it->meta.end() || trim(pw_it->second).empty()) {
        response.status_code = 403;
        response.status_text = "Forbidden";
        response.body = "{\"error\":\"post is not protected\"}";
        return true;
    }

    if (trim(pw_it->second) != password) {
        response.status_code = 403;
        response.status_text = "Forbidden";
        response.body = "{\"error\":\"invalid password\"}";
        return true;
    }

    response.status_code = 200;
    response.status_text = "OK";
    response.body = "{\"ok\":true,\"html\":\"" + escape_json(post_it->html) + "\"}";
    return true;
}

}  // namespace blogpp
