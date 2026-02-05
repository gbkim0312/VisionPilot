#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "out_adapter_registry.hpp"
#include "vslam_config.hpp"

namespace vp::assembly
{
OutAdapterRegistry::OutAdapterRegistry(const config::AssemblyConfig &config)
    : config_(config)
{
    LOG_TRA("");

    switch (config_.vslamAdapterConfig.type)
    {
    case config::VslamType::STELLA_VSLAM:
        LOG_INF("Stella VSLAM adapter selected.");
        stella_vslam_adapter_ = std::make_unique<vp::adapter::out::StellaVslamAdapter>(config_.vslamAdapterConfig);
        break;
    case config::VslamType::ORB_SLAM3:
        LOG_INF("ORB SLAM3 adapter selected.");
        orb_slam_adapter_ = std::make_unique<vp::adapter::out::OrbSlamAdapter>(config_.vslamAdapterConfig);
        break;
    case config::VslamType::NONE:
        no_vslam_adapter_ = std::make_unique<vp::adapter::out::NoVslamAdapter>();
        LOG_INF("No VSLAM adapter selected.");
        break;
    default:
        THROWLOG(SysException, "Unsupported VSLAM type specified in configuration.");
    }

    switch (config_.viewerConfig.viewerType)
    {
    case config::ViewerType::NONE:
        LOG_INF("No VSLAM viewer selected.");
        none_viewer_adapter_ = std::make_unique<vp::adapter::out::NoneViewerAdapter>(config_.viewerConfig);
        break;
    case config::ViewerType::OPENCV:
        LOG_INF("OpenCV VSLAM viewer selected.");
        opencv_viewer_adapter_ = std::make_unique<vp::adapter::out::OpenCVViewerAdapter>(config_.viewerConfig);
        break;
    case config::ViewerType::PANGOLIN:
        LOG_INF("Pangolin VSLAM viewer selected.");
        pangolin_viewer_adapter_ = std::make_unique<vp::adapter::out::PangolinViewerAdapter>(config_.viewerConfig);
        break;
    case config::ViewerType::SOCKET:
        LOG_INF("Socket VSLAM viewer selected.");
        socket_viewer_adapter_ = std::make_unique<vp::adapter::out::SocketViewerAdapter>(config_.viewerConfig);
        break;
    default:
        THROWLOG(SysException, "Unsupported VSLAM viewer type specified in configuration.");
    }

    switch (config_.detectionConfig.type)
    {
    case config::DetectionType::YOLOV8:
        LOG_INF("YOLOv8 detection adapter selected.");
        yolo_v8_adapter_ = std::make_unique<vp::adapter::out::YOLOv8Adapter>(config_.detectionConfig);
        break;
    case config::DetectionType::NONE:
        LOG_INF("No detection adapter selected.");
        no_detection_adapter_ = std::make_unique<vp::adapter::out::NoDetectionAdapter>();
        break;
    default:
        THROWLOG(SysException, "Unsupported Detection type specified in configuration.");
    }
}

void OutAdapterRegistry::startExternalAdapters()
{
    LOG_TRA("");
    switch (config_.viewerConfig.viewerType)
    {
    case config::ViewerType::NONE:
        none_viewer_adapter_->start();
        break;
    case config::ViewerType::OPENCV:
        opencv_viewer_adapter_->start();
        break;
    case config::ViewerType::PANGOLIN:
        pangolin_viewer_adapter_->start();
        break;
    case config::ViewerType::SOCKET:
        socket_viewer_adapter_->start();
        break;
    default:
        LOG_WRN("Unsupported VSLAM viewer type. No viewer will be started.");
        break;
    }

    switch (config_.detectionConfig.type)
    {
    case config::DetectionType::YOLOV8:
        yolo_v8_adapter_->initialize();
        break;
    case config::DetectionType::NONE:
        no_detection_adapter_->initialize();
        break;
    default:
        LOG_WRN("Unsupported Detection type. No object detection adapter will be started.");
        break;
    }

    switch (config_.vslamAdapterConfig.type)
    {
    case config::VslamType::STELLA_VSLAM:
        stella_vslam_adapter_->start();
        break;
    case config::VslamType::ORB_SLAM3:
        orb_slam_adapter_->initialize();
        break;
    case config::VslamType::NONE:
        break;
    default:
        LOG_WRN("Unsupported VSLAM method. No localization adapter will be started.");
        break;
    }
}

void OutAdapterRegistry::stopExternalAdapters()
{
    LOG_TRA("");

    switch (config_.detectionConfig.type)
    {
    case config::DetectionType::YOLOV8:
        yolo_v8_adapter_->deinitialize();
        break;
    case config::DetectionType::NONE:
        break;
    default:
        LOG_WRN("Unsupported Detection type. No object detection adapter to stop.");
        break;
    }

    switch (config_.vslamAdapterConfig.type)
    {
    case config::VslamType::STELLA_VSLAM:
        stella_vslam_adapter_->stop();
        break;
    case config::VslamType::ORB_SLAM3:
        orb_slam_adapter_->deinitialize();
        break;
    default:
        break;
    }

    switch (config_.viewerConfig.viewerType)
    {
    case config::ViewerType::NONE:
        none_viewer_adapter_->stop();
        break;
    case config::ViewerType::OPENCV:
        opencv_viewer_adapter_->stop();
        break;
    case config::ViewerType::PANGOLIN:
        pangolin_viewer_adapter_->stop();
        break;
    case config::ViewerType::SOCKET:
        socket_viewer_adapter_->stop();
        break;
    default:
        break;
    }
}

vp::port::out::LocalizationPort &OutAdapterRegistry::getLocalizationPort()
{
    switch (config_.vslamAdapterConfig.type)
    {
    case config::VslamType::STELLA_VSLAM:
        return *stella_vslam_adapter_;
    case config::VslamType::ORB_SLAM3:
        return *orb_slam_adapter_;
    case config::VslamType::NONE:
        return *no_vslam_adapter_;
    default:
        THROWLOG(SysException, "Unsupported VSLAM method. Cannot provide LocalizationPort.");
    }
}

vp::port::out::VisualizationPort &OutAdapterRegistry::getVisualizationPort()
{
    switch (config_.viewerConfig.viewerType)
    {
    case config::ViewerType::NONE:
        return *none_viewer_adapter_;
    case config::ViewerType::OPENCV:
        return *opencv_viewer_adapter_;
    case config::ViewerType::PANGOLIN:
        return *pangolin_viewer_adapter_;
    case config::ViewerType::SOCKET:
        return *socket_viewer_adapter_;
    default:
        THROWLOG(SysException, "Unsupported VSLAM viewer type. Cannot provide VisualizationPort.");
    }
}

vp::port::out::ObjectDetectionPort &OutAdapterRegistry::getObjectDetectionPort()
{
    switch (config_.detectionConfig.type)
    {
    case config::DetectionType::YOLOV8:
        return *yolo_v8_adapter_;
    case config::DetectionType::NONE:
        return *no_detection_adapter_;
    default:
        THROWLOG(SysException, "Unsupported Detection type. Cannot provide ObjectDetectionPort.");
    }
}
} // namespace vp::assembly
