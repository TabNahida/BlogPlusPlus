#pragma once

#include "plugins/plugin.h"

namespace blogpp {

class PostAuthApiPlugin : public ServerPlugin {
public:
    std::string name() const override;
    bool handle(const HttpRequest& request, HttpResponse& response, SiteBuilder& builder) override;
};

}  // namespace blogpp
