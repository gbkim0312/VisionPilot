#pragma once
#include "video_loader_config.hpp"
#include "vslam_config.hpp"
#include "yolov8_config.hpp"

namespace vp::config
{
struct AssemblyConfig
{
    VideoLoaderConfig videoLoaderConfig;
    VslamAdapterConfig vslamAdapterConfig;
    VslamViewerConfig vslamViewerConfig;
    YoloConfig yoloConfig;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AssemblyConfig,
                                   videoLoaderConfig,
                                   vslamAdapterConfig,
                                   vslamViewerConfig,
                                   yoloConfig)
} // namespace vp::config