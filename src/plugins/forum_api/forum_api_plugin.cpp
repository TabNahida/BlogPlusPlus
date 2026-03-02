#include "plugins/forum_api/forum_api_plugin.h"

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

std::string threads_to_json(const std::vector<ForumApiPlugin::ForumThread>& threads) {
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

std::string posts_to_json(const std::vector<ForumApiPlugin::ForumPost>& posts) {
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

std::string message_compat_json(const std::vector<ForumApiPlugin::ForumPost>& posts) {
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < posts.size(); ++i) {
        const auto& post = posts[i];
        out << "  {\n"
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

}  // namespace

ForumApiPlugin::ForumApiPlugin() {
    const auto now = current_datetime();
    threads_.push_back({next_thread_id_++, "Welcome to Forum", "BlogPlusPlus", now});
    posts_.push_back({next_post_id_++, 1, "BlogPlusPlus", "Forum API is running. Start your thread.", now});
    users_.insert("BlogPlusPlus");
}

std::string ForumApiPlugin::name() const {
    return "forum_api";
}

bool ForumApiPlugin::handle(const HttpRequest& request, HttpResponse& response, SiteBuilder&) {
    const auto method = to_lower(request.method);
    response.content_type = "application/json; charset=utf-8";

    if (request.path == "/api/forum/threads") {
        if (method == "get") {
            response.status_code = 200;
            response.status_text = "OK";
            response.body = threads_to_json(threads_);
            return true;
        }
        if (method == "post") {
            const auto form = parse_form_urlencoded(request.body);
            auto title = trim(form.count("title") ? form.at("title") : "");
            auto author = trim(form.count("author") ? form.at("author") : "Anonymous");
            auto text = trim(form.count("text") ? form.at("text") : "");

            if (title.empty()) {
                response.status_code = 400;
                response.status_text = "Bad Request";
                response.body = "{\"error\":\"title is required\"}";
                return true;
            }

            if (author.empty()) author = "Anonymous";
            if (author.size() > 40) author = author.substr(0, 40);
            if (title.size() > 120) title = title.substr(0, 120);
            if (text.size() > 360) text = text.substr(0, 360);

            const int thread_id = next_thread_id_++;
            const auto created = current_datetime();
            threads_.push_back({thread_id, title, author, created});
            users_.insert(author);

            if (!text.empty()) {
                posts_.push_back({next_post_id_++, thread_id, author, text, created});
            }

            response.status_code = 201;
            response.status_text = "Created";
            response.body = "{\"ok\":true,\"thread_id\":" + std::to_string(thread_id) + "}";
            return true;
        }

        response.status_code = 405;
        response.status_text = "Method Not Allowed";
        response.body = "{\"error\":\"method not allowed\"}";
        return true;
    }

    if (request.path == "/api/forum/posts") {
        if (method == "get") {
            int thread_id = 0;
            if (request.query_params.count("thread_id")) {
                try {
                    thread_id = std::stoi(request.query_params.at("thread_id"));
                } catch (...) {
                    thread_id = 0;
                }
            }

            std::vector<ForumPost> result;
            if (thread_id <= 0) {
                result = posts_;
            } else {
                for (const auto& post : posts_) {
                    if (post.thread_id == thread_id) {
                        result.push_back(post);
                    }
                }
            }

            response.status_code = 200;
            response.status_text = "OK";
            response.body = posts_to_json(result);
            return true;
        }

        if (method == "post") {
            const auto form = parse_form_urlencoded(request.body);
            int thread_id = 0;
            try {
                thread_id = std::stoi(form.count("thread_id") ? form.at("thread_id") : "0");
            } catch (...) {
                thread_id = 0;
            }
            auto author = trim(form.count("author") ? form.at("author") : "Anonymous");
            auto text = trim(form.count("text") ? form.at("text") : "");

            if (thread_id <= 0 || text.empty()) {
                response.status_code = 400;
                response.status_text = "Bad Request";
                response.body = "{\"error\":\"thread_id and text are required\"}";
                return true;
            }

            const auto thread_it = std::find_if(threads_.begin(), threads_.end(), [&](const ForumThread& thread) {
                return thread.id == thread_id;
            });
            if (thread_it == threads_.end()) {
                response.status_code = 404;
                response.status_text = "Not Found";
                response.body = "{\"error\":\"thread not found\"}";
                return true;
            }

            if (author.empty()) author = "Anonymous";
            if (author.size() > 40) author = author.substr(0, 40);
            if (text.size() > 360) text = text.substr(0, 360);

            posts_.push_back({next_post_id_++, thread_id, author, text, current_datetime()});
            if (posts_.size() > 2000) {
                posts_.erase(posts_.begin(), posts_.begin() + static_cast<long long>(posts_.size() - 2000));
            }
            users_.insert(author);

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

    if (request.path == "/api/forum/users") {
        if (method != "get") {
            response.status_code = 405;
            response.status_text = "Method Not Allowed";
            response.body = "{\"error\":\"method not allowed\"}";
            return true;
        }
        response.status_code = 200;
        response.status_text = "OK";
        response.body = users_to_json(users_);
        return true;
    }

    // Compatibility endpoint for older forum UI.
    if (request.path == "/api/forum/messages") {
        if (method == "get") {
            response.status_code = 200;
            response.status_text = "OK";
            response.body = message_compat_json(posts_);
            return true;
        }
        if (method == "post") {
            const auto form = parse_form_urlencoded(request.body);
            auto author = trim(form.count("author") ? form.at("author") : "Anonymous");
            auto text = trim(form.count("text") ? form.at("text") : "");
            if (text.empty()) {
                response.status_code = 400;
                response.status_text = "Bad Request";
                response.body = "{\"error\":\"text is required\"}";
                return true;
            }
            if (author.empty()) author = "Anonymous";
            if (author.size() > 40) author = author.substr(0, 40);
            if (text.size() > 360) text = text.substr(0, 360);

            int thread_id = threads_.empty() ? 1 : threads_.front().id;
            if (threads_.empty()) {
                threads_.push_back({next_thread_id_++, "General", "BlogPlusPlus", current_datetime()});
                thread_id = threads_.front().id;
            }
            posts_.push_back({next_post_id_++, thread_id, author, text, current_datetime()});
            users_.insert(author);
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

    return false;
}

}  // namespace blogpp
