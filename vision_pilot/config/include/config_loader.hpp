#pragma once
#include "assembly_config.hpp"
#include <string>

namespace vp::config
{
class ConfigLoader
{
public:
    bool loadConfig(const std::string &config_path);

private:
    void loadConfigFromFile(const std::string &config_path);
    config::AssemblyConfig assembly_config_;
};
} // namespace vp::config
