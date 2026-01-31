#include "stereo_vslam_adapter.hpp"
#include <stella_vslam/system.h>

namespace vp::adapter::out
{
class StereoVSlamAdapterImpl
{
public:
    StereoVSlamAdapterImpl(const config::VslamAdapterConfig &vslam_config);
    ~StereoVSlamAdapterImpl();

    bool initialize();
    domain::model::Pose update(const domain::model::ImagePacket &image, uint64_t timestamp);
    bool stop();

private:
    const config::VslamAdapterConfig &vslam_config_;

    std::shared_ptr<stella_vslam::system> slam_system_;
    bool is_initialized_ = false;
};
} // namespace vp::adapter::out