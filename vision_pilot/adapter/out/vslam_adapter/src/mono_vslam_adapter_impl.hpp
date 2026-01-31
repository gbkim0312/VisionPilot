#pragma once
#include "image.hpp"
#include "mono_vslam_adapter.hpp"
#include "pose.hpp"
#include "vslam_config.hpp"
#include <stella_vslam/system.h>

namespace vp::adapter::out
{
class MonoVSlamAdapterImpl
{
public:
    MonoVSlamAdapterImpl(const config::VslamAdapterConfig &vslam_config);
    ~MonoVSlamAdapterImpl();

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp);

private:
    const config::VslamAdapterConfig &vslam_config_;
    uint32_t _ = 0;

    std::shared_ptr<stella_vslam::system> slam_system_;
    bool is_initialized_ = false;
};
} // namespace vp::adapter::out
