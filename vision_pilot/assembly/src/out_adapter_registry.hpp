#pragma once

#include "assembly_config.hpp"
#include "mono_vslam_adapter.hpp"
#include "no_slam_adapter.hpp"
#include "stereo_vslam_adapter.hpp"

#include "no_detection_adapter.hpp"
#include "yolov8_adapter.hpp"

#include "none_viewer_adapter.hpp"
#include "opencv_viewer_adapter.hpp"
#include "pangolin_viewer_adapter.hpp"
#include "socket_viewer_adapter.hpp"

namespace vp::assembly
{
class OutAdapterRegistry
{
public:
    explicit OutAdapterRegistry(const config::AssemblyConfig &config);

    void startExternalAdapters();
    void stopExternalAdapters();

    vp::port::out::LocalizationPort &getLocalizationPort();
    vp::port::out::VisualizationPort &getVisualizationPort();
    vp::port::out::ObjectDetectionPort &getObjectDetectionPort();

private:
    const config::AssemblyConfig &config_;

    std::unique_ptr<vp::adapter::out::MonoVSlamAdapter> mono_vslam_adapter_;
    std::unique_ptr<vp::adapter::out::StereoVSlamAdapter> stereo_vslam_adapter_;
    std::unique_ptr<vp::adapter::out::NoSlamAdapter> no_slam_adapter_;

    std::unique_ptr<vp::adapter::out::NoneViewerAdapter> none_viewer_adapter_;
    std::unique_ptr<vp::adapter::out::OpenCVViewerAdapter> opencv_viewer_adapter_;
    std::unique_ptr<vp::adapter::out::PangolinViewerAdapter> pangolin_viewer_adapter_;
    std::unique_ptr<vp::adapter::out::SocketViewerAdapter> socket_viewer_adapter_;

    std::unique_ptr<vp::adapter::out::YOLOv8Adapter> yolo_v8_adapter_;
    std::unique_ptr<vp::adapter::out::NoDetectionAdapter> no_detection_adapter_;
};
} // namespace vp::assembly