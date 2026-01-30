#include "config_loader.hpp"
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

void ConfigLoader::loadConfigFromFile(const std::string &config_path)
{
    LOG_TRA("");

    assembly_config_ = jsonFileToStruct<config::AssemblyConfig>(config_path);
}

} // namespace vp::config