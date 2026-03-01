#pragma once

#include "plugins/plugin.h"

#include <vector>

namespace blogpp {

class ForumApiPlugin : public ServerPlugin {
public:
    struct ForumMessage {
        std::string author;
        std::string text;
        std::string created_at;
    };

    ForumApiPlugin();

    std::string name() const override;
    bool handle(const HttpRequest& request, HttpResponse& response, SiteBuilder& builder) override;

private:
    std::vector<ForumMessage> messages_;
};

}  // namespace blogpp
