#pragma once

#include "detection.hpp"
#include "image.hpp"
#include "pose.hpp"
#include "viewer_config.hpp"
#include <pangolin/pangolin.h>
#include <vector>

namespace vp::adapter::out
{
class PangolinViewerAdapterImpl
{
public:
    explicit PangolinViewerAdapterImpl(const config::ViewerConfig &config);
    ~PangolinViewerAdapterImpl();

    bool start();
    bool stop();

    void render(const domain::model::Pose &pose,
                const std::vector<domain::model::Detection> &detections,
                const domain::model::ImagePacket &frame);

private:
    void drawTrajectory();
    void drawCurrentPose(const domain::model::Pose &pose);
    void drawGrid();

    config::ViewerConfig config_;
    bool is_running_ = false;

    std::string window_name_ = "Vision Pilot - Pangolin Viewer";

    std::unique_ptr<pangolin::OpenGlRenderState> s_cam_;
    pangolin::View *d_cam_ = nullptr;

    std::vector<domain::model::Pose> pose_history_{};
    const size_t max_history_size_ = 1000;

    std::thread render_thread_;
    std::mutex data_mutex_;
    domain::model::Pose latest_pose_;
    bool has_pose_ = false;
};
} // namespace vp::adapter::out