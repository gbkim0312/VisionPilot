#include "stereo_vslam_adapter_impl.hpp"

namespace vp::adapter::out
{
StereoVSlamAdapterImpl::StereoVSlamAdapterImpl(const config::VslamAdapterConfig &vslam_config)
    : vslam_config_(vslam_config)
{
}
StereoVSlamAdapterImpl::~StereoVSlamAdapterImpl() = default;

bool StereoVSlamAdapterImpl::initialize()
{
    if (is_initialized_)
    {
        return true;
    }

    is_initialized_ = true;
    return is_initialized_;
}

domain::model::Pose StereoVSlamAdapterImpl::update(const domain::model::ImagePacket & /* image */, uint64_t /* timestamp */)
{
    domain::model::Pose pose;

    return pose;
}

bool StereoVSlamAdapterImpl::stop()
{
    is_initialized_ = false;
    return true;
}

} // namespace vp::adapter::out