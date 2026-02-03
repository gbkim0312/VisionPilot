#include "orb_slam3_adapter.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "orb_slam3_adapter_impl.hpp"

namespace vp::adapter::out
{
OrbSlamAdapter::OrbSlamAdapter(const config::VslamAdapterConfig &config)
{
    LOG_TRA("");

    if (config.type != config::VslamType::ORB_SLAM3)
    {
        THROWLOG(SysException, "OrbSlamAdapter initialized with unsupported VSLAM type.");
    }

    impl_ = std::make_unique<OrbSlamAdapterImpl>(config);
}

OrbSlamAdapter::~OrbSlamAdapter() = default;

bool OrbSlamAdapter::initialize()
{
    return impl_->initialize();
}

domain::model::Pose OrbSlamAdapter::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    return impl_->update(image, timestamp);
}

bool OrbSlamAdapter::deinitialize()
{
    return impl_->deinitialize();
}
} // namespace vp::adapter::out