#pragma once

#include "plugins/plugin.h"

namespace blogpp {

class ArchivePlugin : public Plugin {
public:
    std::string name() const override;
    void apply(SiteBuilder& builder) override;
};

}  // namespace blogpp

