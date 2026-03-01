#pragma once

#include "core/http_types.h"

#include <string>

namespace blogpp {

class SiteBuilder;

class Plugin {
public:
    virtual ~Plugin() = default;
    virtual std::string name() const = 0;
    virtual void apply(SiteBuilder& builder) = 0;
};

class ServerPlugin {
public:
    virtual ~ServerPlugin() = default;
    virtual std::string name() const = 0;
    virtual bool handle(const HttpRequest& request, HttpResponse& response, SiteBuilder& builder) = 0;
};

}  // namespace blogpp
