#pragma once

#include "detection.hpp"
#include "image.hpp"
#include "pose.hpp"
#include "tracking.hpp"
#include "viewer_config.hpp"
namespace vp::adapter::out
{
class OpenCVViewerAdapterImpl
{
public:
    OpenCVViewerAdapterImpl(const config::ViewerConfig &config);
    ~OpenCVViewerAdapterImpl();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, const domain::model::DetectionResult &detections, const domain::model::TrackingResult &tracking, const domain::model::ImagePacket &frame);

private:
    const config::ViewerConfig config_;
    const std::string window_name_ = "VisionPilot - OpenCV Viewer";
};
} // namespace vp::adapter::out