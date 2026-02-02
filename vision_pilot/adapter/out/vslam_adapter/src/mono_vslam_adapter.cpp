#include "mono_vslam_adapter.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "mono_vslam_adapter_impl.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{
MonoVSlamAdapter::MonoVSlamAdapter(const config::VslamAdapterConfig &vslam_config)
{
    LOG_TRA("");

    if (vslam_config.method != config::VslamMethod::MONOCULAR)
    {
        THROWLOG(SysException, "Type mismatch: MonoVSlamAdapterImpl can be used only with MONOCULAR VSLAM method.");
    }

    impl_ = std::make_unique<MonoVSlamAdapterImpl>(vslam_config);
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

bool MonoVSlamAdapter::deinitialize()
{
    return impl_->stop();
}
} // namespace vp::adapter::out
