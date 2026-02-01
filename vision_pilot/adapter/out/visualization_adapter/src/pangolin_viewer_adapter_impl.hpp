#pragma once

#include "detection.hpp"
#include "image.hpp"
#include "pose.hpp"
#include "vslam_config.hpp"
#include <vector>

namespace vp::adapter::out
{
class PangolinViewerAdapterImpl
{
public:
    explicit PangolinViewerAdapterImpl(const config::VslamViewerConfig &config);
    ~PangolinViewerAdapterImpl();
    bool start();
    bool stop();
    void render(const domain::model::Pose &pose, std::vector<domain::model::Detection> detections, const domain::model::ImagePacket &frame);

private:
    config::VslamViewerConfig config_;
    bool is_running_ = false;

    std::string window_name_ = "Vision Pilot - Pangolin Viewer";
    // std::unique_ptr<pangolin::OpenGlRenderState> s_cam_ = nullptr;
    // pangolin::View *d_cam_ = nullptr;

    std::vector<domain::model::Pose> pose_history_{};
    const size_t max_history_size_ = 1000;
    void drawTrajectory();
};
} // namespace vp::adapter::out