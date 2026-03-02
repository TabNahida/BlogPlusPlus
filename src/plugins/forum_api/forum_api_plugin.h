#pragma once

#include "plugins/plugin.h"

#include <set>
#include <vector>

namespace blogpp {

class ForumApiPlugin : public ServerPlugin {
public:
    struct ForumThread {
        int id = 0;
        std::string title;
        std::string author;
        std::string created_at;
    };

    struct ForumPost {
        int id = 0;
        int thread_id = 0;
        std::string author;
        std::string text;
        std::string created_at;
    };

    ForumApiPlugin();

    std::string name() const override;
    bool handle(const HttpRequest& request, HttpResponse& response, SiteBuilder& builder) override;

private:
    int next_thread_id_ = 1;
    int next_post_id_ = 1;
    std::vector<ForumThread> threads_;
    std::vector<ForumPost> posts_;
    std::set<std::string> users_;
};

}  // namespace blogpp
