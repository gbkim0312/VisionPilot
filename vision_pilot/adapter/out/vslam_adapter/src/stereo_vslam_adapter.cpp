#include "stereo_vslam_adapter.hpp"
#include "gaia_log.hpp"
#include "stereo_vslam_adapter_impl.hpp"

namespace vp::adapter::out
{
StereoVSlamAdapter::StereoVSlamAdapter(const config::VslamAdapterConfig &vslam_config)
    : impl_(std::make_unique<StereoVSlamAdapterImpl>(vslam_config))
{
}

StereoVSlamAdapter::~StereoVSlamAdapter()
{
    LOG_TRA("");
}

bool StereoVSlamAdapter::initialize()
{
    return impl_->initialize();
}
domain::model::Pose StereoVSlamAdapter::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    return impl_->update(image, timestamp);
}

bool StereoVSlamAdapter::stop()
{
    return impl_->stop();
}

} // namespace vp::adapter::out