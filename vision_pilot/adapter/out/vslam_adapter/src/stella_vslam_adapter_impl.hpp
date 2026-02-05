#pragma once

#include "gaia_circular_queue.hpp"
#include "image.hpp"
#include "pangolin_viewer/viewer.h"
#include "pose.hpp"
#include "stella_vslam_adapter.hpp"
#include "vslam_config.hpp"

#include <memory>
#include <stella_vslam/system.h>
#include <stella_vslam/type.h>
#include <thread>

namespace vp::adapter::out
{
struct ImageTimestamp
{
    domain::model::ImagePacket image;
    uint64_t timestamp;
};

class StellaVslamAdapterImpl
{
public:
    StellaVslamAdapterImpl(const config::VslamAdapterConfig &vslam_config);
    ~StellaVslamAdapterImpl();

    bool start();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp);
    bool stop();

private:
    const config::VslamAdapterConfig &vslam_config_;
    std::shared_ptr<stella_vslam::system> slam_system_ = nullptr;
    std::unique_ptr<pangolin_viewer::viewer> viewer_ = nullptr;
    std::thread viewer_thread_;
    std::thread slam_thread_;

    ThreadSafeCircularQueue<ImageTimestamp> frame_queue_;

    std::atomic_bool is_running_ = false;
    bool is_able_to_save_ = false;
    domain::model::Pose last_pose_{};
    std::mutex pose_mutex_;

    void runSlam();

    void makeOutputDirectory();
    bool initializeViewer();

    domain::model::Pose feedFrame(const domain::model::ImagePacket &image, uint64_t timestamp);
    domain::model::Pose feedMonoFrame(const domain::model::ImagePacket &image, uint64_t timestamp);
    domain::model::Pose feedStereoFrame(const domain::model::ImagePacket &image, uint64_t timestamp);
    domain::model::Pose feedRgbdFrame(const domain::model::ImagePacket &image, uint64_t timestamp);

    domain::model::Pose convertStellaPoseToDomainPose(const std::shared_ptr<stella_vslam::Mat44_t> &raw_pose);

    bool saveResults();
};
} // namespace vp::adapter::out
