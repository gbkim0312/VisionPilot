#include "vslam_adapter.hpp"
#include "gaia_log.hpp"
#include "vslam_adapter_impl.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{
VslamAdapter::VslamAdapter(const config::VslamAdapterConfig &vslam_config)
    : impl_(std::make_unique<VslamAdapterImpl>(vslam_config))
{
}

VslamAdapter::~VslamAdapter()
{
    LOG_TRA("");
}

bool VslamAdapter::initialize()
{
    return impl_->initialize();
}

domain::model::Pose VslamAdapter::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    return impl_->update(image, timestamp);
}
} // namespace vp::adapter::out
