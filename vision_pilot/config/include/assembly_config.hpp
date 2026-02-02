#pragma once
#include "detection_config.hpp"
#include "video_loader_config.hpp"
#include "vslam_config.hpp"

namespace vp::config
{
struct AssemblyConfig
{
    VideoLoaderConfig videoLoaderConfig;
    VslamAdapterConfig vslamAdapterConfig;
    VslamViewerConfig vslamViewerConfig;
    DetectionConfig detectionConfig;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AssemblyConfig,
                                   videoLoaderConfig,
                                   vslamAdapterConfig,
                                   vslamViewerConfig,
                                   detectionConfig)
} // namespace vp::config