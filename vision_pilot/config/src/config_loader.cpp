#include "config_loader.hpp"
#include "assembly_config.hpp"
#include "gaia_json_util.hpp"
#include "gaia_log.hpp"

namespace vp::config
{
bool ConfigLoader::loadConfig(const std::string &config_path)
{
    LOG_INF("Loading configuration from path: {}", config_path);
    // 구성 파일 로드 로직 구현

    this->loadConfigFromFile(config_path);
    return true;
}

bool ConfigLoader::isLoaded() const
{
    LOG_TRA("");

    return is_loaded_;
}

const AssemblyConfig &ConfigLoader::getAssemblyConfig() const
{
    LOG_TRA("");

    return assembly_config_;
}

void ConfigLoader::printConfig() const
{
    LOG_TRA("");

    auto json_str = structToJsonStr(assembly_config_);
    LOG_INF("Current Assembly Configuration: {}", json_str);
}

void ConfigLoader::loadConfigFromFile(const std::string &config_path)
{
    LOG_TRA("");

    assembly_config_ = jsonFileToStruct<config::AssemblyConfig>(config_path);
    is_loaded_ = true;
}

} // namespace vp::config