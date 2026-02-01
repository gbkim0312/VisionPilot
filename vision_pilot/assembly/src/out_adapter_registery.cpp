#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "out_adapter_registry.hpp"

namespace vp::assembly
{
OutAdapterRegistry::OutAdapterRegistry(const config::AssemblyConfig &config)
    : config_(config),
      mono_vslam_adapter_(config_.vslamAdapterConfig),
      stereo_vslam_adapter_(config_.vslamAdapterConfig),
      no_slam_adapter_(config_.vslamAdapterConfig),
      none_viewer_adapter_(config_.vslamViewerConfig),
      opencv_viewer_adapter_(config_.vslamViewerConfig),
      pangolin_viewer_adapter_(config_.vslamViewerConfig),
      socket_viewer_adapter_(config_.vslamViewerConfig),
      yolo_v8_adapter_(config_.yoloConfig)
{
    LOG_TRA("");
}

void OutAdapterRegistry::startExternalAdapters()
{
    LOG_TRA("");
    switch (config_.vslamViewerConfig.viewerType)
    {
    case config::VslamViewerType::NONE:
        none_viewer_adapter_.start();
        break;
    case config::VslamViewerType::OPENCV:
        opencv_viewer_adapter_.start();
        break;
    case config::VslamViewerType::PANGOLIN:
        LOG_WRN("Pangolin viewer is not yet implemented.");
        pangolin_viewer_adapter_.start();
        break;
    case config::VslamViewerType::SOCKET:
        LOG_WRN("Socket viewer is not yet implemented.");
        socket_viewer_adapter_.start();
        break;
    default:
        LOG_WRN("Unsupported VSLAM viewer type. No viewer will be started.");
        break;
    }

    switch (config_.vslamAdapterConfig.method)
    {
    case config::VslamMethod::MONOCULAR:
        mono_vslam_adapter_.initialize();
        break;
    case config::VslamMethod::STEREO:
        stereo_vslam_adapter_.initialize();
        break;
    case config::VslamMethod::DISABLED:
        no_slam_adapter_.initialize();
        break;
    default:
        LOG_WRN("Unsupported VSLAM method. No localization adapter will be started.");
        break;
    }

    yolo_v8_adapter_.initialize();
}

void OutAdapterRegistry::stopExternalAdapters()
{
    LOG_TRA("");

    yolo_v8_adapter_.deinitialize();

    switch (config_.vslamAdapterConfig.method)
    {
    case config::VslamMethod::MONOCULAR:
        mono_vslam_adapter_.deinitialize();
        break;
    case config::VslamMethod::STEREO:
        stereo_vslam_adapter_.deinitialize();
        break;
    case config::VslamMethod::DISABLED:
        no_slam_adapter_.deinitialize();
        break;
    default:
        break;
    }

    switch (config_.vslamViewerConfig.viewerType)
    {
    case config::VslamViewerType::NONE:
        none_viewer_adapter_.stop();
        break;
    case config::VslamViewerType::OPENCV:
        opencv_viewer_adapter_.stop();
        break;
    case config::VslamViewerType::PANGOLIN:
        pangolin_viewer_adapter_.stop();
        break;
    case config::VslamViewerType::SOCKET:
        socket_viewer_adapter_.stop();
        break;
    default:
        break;
    }

    yolo_v8_adapter_.deinitialize();
}

vp::port::out::LocalizationPort &OutAdapterRegistry::getLocalizationPort()
{
    switch (config_.vslamAdapterConfig.method)
    {
    case config::VslamMethod::MONOCULAR:
        return mono_vslam_adapter_;
    case config::VslamMethod::STEREO:
        return stereo_vslam_adapter_;
    case config::VslamMethod::DISABLED:
        return no_slam_adapter_;
    default:
        THROWLOG(SysException, "Unsupported VSLAM method. Cannot provide LocalizationPort.");
    }
}

vp::port::out::VisualizationPort &OutAdapterRegistry::getVisualizationPort()
{
    switch (config_.vslamViewerConfig.viewerType)
    {
    case config::VslamViewerType::NONE:
        return none_viewer_adapter_;
    case config::VslamViewerType::OPENCV:
        return opencv_viewer_adapter_;
    case config::VslamViewerType::PANGOLIN:
        return pangolin_viewer_adapter_;
    case config::VslamViewerType::SOCKET:
        return socket_viewer_adapter_;
    default:
        THROWLOG(SysException, "Unsupported VSLAM viewer type. Cannot provide VisualizationPort.");
    }
}

vp::port::out::ObjectDetectionPort &OutAdapterRegistry::getObjectDetectionPort()
{
    return yolo_v8_adapter_;
}

} // namespace vp::assembly
