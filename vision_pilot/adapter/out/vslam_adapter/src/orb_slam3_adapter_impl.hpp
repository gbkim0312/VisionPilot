#pragma once

#include "orb_slam3_adapter.hpp"
#include <ORB_SLAM3/System.h>

namespace vp::adapter::out
{
class OrbSlamAdapterImpl
{
public:
    OrbSlamAdapterImpl(const config::VslamAdapterConfig &config);
    ~OrbSlamAdapterImpl();

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp);
    bool deinitialize();

private:
    const config::VslamAdapterConfig &config_;
    std::unique_ptr<ORB_SLAM3::System> orb_slam_system_;

    std::atomic_bool is_initialized_{false};

    domain::model::Pose feedMonoFrame(const domain::model::ImagePacket &image, uint64_t timestamp);
    domain::model::Pose feedStereoFrame(const domain::model::ImagePacket &image, uint64_t timestamp);
    domain::model::Pose feedRgbdFrame(const domain::model::ImagePacket &image, uint64_t timestamp);

    domain::model::Pose convertOrbSlamPoseToDomainPose(const Sophus::SE3f &orb_pose);
};
} // namespace vp::adapter::out