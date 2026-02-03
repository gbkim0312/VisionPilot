#pragma once

#include "image.hpp"
#include "pose.hpp"
#include "stella_vslam_adapter.hpp"
#include "vslam_config.hpp"
#include <memory>
#include <stella_vslam/system.h>
#include <stella_vslam/type.h>

namespace vp::adapter::out
{
class StellaVslamAdapterImpl
{
public:
    StellaVslamAdapterImpl(const config::VslamAdapterConfig &vslam_config);
    ~StellaVslamAdapterImpl();

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp);
    bool deinitialize();

private:
    const config::VslamAdapterConfig &vslam_config_;
    std::shared_ptr<stella_vslam::system> slam_system_ = nullptr;
    bool is_initialized_ = false;

    domain::model::Pose feedMonoFrame(const domain::model::ImagePacket &image, uint64_t timestamp);
    domain::model::Pose feedStereoFrame(const domain::model::ImagePacket &image, uint64_t timestamp);
    domain::model::Pose feedRgbdFrame(const domain::model::ImagePacket &image, uint64_t timestamp);

    domain::model::Pose convertStellaPoseToDomainPose(const std::shared_ptr<stella_vslam::Mat44_t> &raw_pose);
};
} // namespace vp::adapter::out
