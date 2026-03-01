#pragma once

#include "plugins/plugin.h"

namespace blogpp {

class MathPlugin : public Plugin {
public:
    std::string name() const override;
    void apply(SiteBuilder& builder) override;
};

}  // namespace blogpp
