#pragma once
#include "assembly_config.hpp"
#include <string>

namespace vp::config
{
class ConfigLoader
{
public:
    bool loadConfig(const std::string &config_path);
    bool isLoaded() const;

    const AssemblyConfig &getAssemblyConfig() const;

private:
    bool is_loaded_{false};
    config::AssemblyConfig assembly_config_;

    void loadConfigFromFile(const std::string &config_path);
};
} // namespace vp::config
