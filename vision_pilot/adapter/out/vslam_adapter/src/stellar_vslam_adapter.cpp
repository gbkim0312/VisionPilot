#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "stella_vslam_adapter.hpp"
#include "stella_vslam_adapter_impl.hpp"
namespace vp::adapter::out
{
StellaVslamAdapter::StellaVslamAdapter(const config::VslamAdapterConfig &vslam_config)
{
    LOG_TRA("");
    if (vslam_config.type != config::VslamType::STELLA_VSLAM)
    {
        THROWLOG(SysException, "Type mismatch: StellaVslamAdapter can be used only with STELLA_VSLAM type.");
    }
    impl_ = std::make_unique<StellaVslamAdapterImpl>(vslam_config);
}

StellaVslamAdapter::~StellaVslamAdapter()
{
    LOG_TRA("");
}

bool StellaVslamAdapter::initialize()
{
    return impl_->initialize();
}

domain::model::Pose StellaVslamAdapter::update(const domain::model::ImagePacket &image, uint64_t timestamp)
{
    return impl_->update(image, timestamp);
}

bool StellaVslamAdapter::deinitialize()
{
    return impl_->deinitialize();
}

} // namespace vp::adapter::out
