#pragma once
#include <string>

namespace vp::config
{
class ConfigLoader
{
public:
    bool loadConfig(const std::string &configPath);
};
} // namespace vp::config
