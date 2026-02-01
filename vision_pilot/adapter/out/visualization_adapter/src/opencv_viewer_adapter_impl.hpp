#pragma once

#include "detection.hpp"
#include "image.hpp"
#include "pose.hpp"
#include "vslam_config.hpp"
namespace vp::adapter::out
{
class OpenCVViewerAdapterImpl
{
public:
    OpenCVViewerAdapterImpl(const config::VslamViewerConfig &config);
    ~OpenCVViewerAdapterImpl();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, std::vector<domain::model::Detection> detections, const domain::model::ImagePacket &frame);

private:
    const config::VslamViewerConfig config_;
    const std::string window_name_ = "VisionPilot - OpenCV Viewer";
};
} // namespace vp::adapter::out