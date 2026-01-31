#pragma once
#include "vslam_config.hpp"
#include <video_loader_config.hpp>

namespace vp::config
{
struct AssemblyConfig
{
    VideoLoaderConfig videoLoaderConfig;
    VslamAdapterConfig vslamAdapterConfig;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AssemblyConfig,
                                   videoLoaderConfig,
                                   vslamAdapterConfig)
} // namespace vp::config