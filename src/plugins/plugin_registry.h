#pragma once

#include "plugins/plugin.h"

#include <memory>
#include <vector>

namespace blogpp {

std::vector<std::unique_ptr<Plugin>> create_static_plugins();
std::vector<std::unique_ptr<ServerPlugin>> create_server_plugins();

}  // namespace blogpp
