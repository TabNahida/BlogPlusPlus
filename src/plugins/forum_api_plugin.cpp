#include "plugins/forum_api_plugin.h"

#include "core/site_builder.h"
#include "core/utils.h"

#include <algorithm>
#include <sstream>
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

std::string messages_to_json(const std::vector<ForumApiPlugin::ForumMessage>& messages) {
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& message = messages[i];
        out << "  {\n"
            << "    \"author\": \"" << escape_json(message.author) << "\",\n"
            << "    \"text\": \"" << escape_json(message.text) << "\",\n"
            << "    \"created_at\": \"" << escape_json(message.created_at) << "\"\n"
            << "  }";
        if (i + 1 < messages.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]";
    return out.str();
}

}  // namespace

ForumApiPlugin::ForumApiPlugin() {
    messages_.push_back({"BlogPlusPlus", "Forum API is running. Start discussing here.", current_datetime()});
}

std::string ForumApiPlugin::name() const {
    return "forum_api";
}

bool ForumApiPlugin::handle(const HttpRequest& request, HttpResponse& response, SiteBuilder&) {
    if (request.path != "/api/forum/messages") {
        return false;
    }

    const auto method = to_lower(request.method);
    response.content_type = "application/json; charset=utf-8";

    if (method == "get") {
        response.status_code = 200;
        response.status_text = "OK";
        response.body = messages_to_json(messages_);
        return true;
    }

    if (method == "post") {
        const auto form = parse_form_urlencoded(request.body);
        auto author = trim(form.count("author") ? form.at("author") : "Anonymous");
        auto text = trim(form.count("text") ? form.at("text") : "");

        if (author.empty()) {
            author = "Anonymous";
        }
        if (author.size() > 40) {
            author = author.substr(0, 40);
        }
        if (text.size() > 360) {
            text = text.substr(0, 360);
        }

        if (text.empty()) {
            response.status_code = 400;
            response.status_text = "Bad Request";
            response.body = "{\"error\":\"text is required\"}";
            return true;
        }

        messages_.push_back({author, text, current_datetime()});
        if (messages_.size() > 200) {
            messages_.erase(messages_.begin(), messages_.begin() + static_cast<long long>(messages_.size() - 200));
        }

        response.status_code = 201;
        response.status_text = "Created";
        response.body = "{\"ok\":true}";
        return true;
    }

    response.status_code = 405;
    response.status_text = "Method Not Allowed";
    response.body = "{\"error\":\"method not allowed\"}";
    return true;
}

}  // namespace blogpp
