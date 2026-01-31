#include "mono_vslam_adapter.hpp"
#include "gaia_log.hpp"
#include "mono_vslam_adapter_impl.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{
MonoVSlamAdapter::MonoVSlamAdapter(const config::VslamAdapterConfig &vslam_config)
    : impl_(std::make_unique<MonoVSlamAdapterImpl>(vslam_config))
{
}

MonoVSlamAdapter::~MonoVSlamAdapter()
{
    LOG_TRA("");
}

bool MonoVSlamAdapter::initialize()
{
    return impl_->initialize();
}

domain::model::Pose MonoVSlamAdapter::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    return impl_->update(image, timestamp);
}
} // namespace vp::adapter::out
