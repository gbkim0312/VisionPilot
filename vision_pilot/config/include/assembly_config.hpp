#pragma once
#include <video_loader_config.hpp>

namespace vp::config
{
struct AssemblyConfig
{
    VideoLoaderConfig videoLoaderConfig;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AssemblyConfig,
                                   videoLoaderConfig)
} // namespace vp::config