#include "none_viewer_adapter_impl.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "none_viewer_adapter.hpp"

namespace vp::adapter::out
{
NoneViewerAdapterImpl::NoneViewerAdapterImpl(const config::VslamViewerConfig &config)
{
    LOG_TRA("");

    if (config.viewerType != config::VslamViewerType::NONE)
    {
        THROWLOG(SysException, "VslamViewerType is not NONE");
    }
}

NoneViewerAdapterImpl::~NoneViewerAdapterImpl()
{
    LOG_TRA("");
}

bool NoneViewerAdapterImpl::start()
{
    LOG_TRA("");

    return true;
}
bool NoneViewerAdapterImpl::stop()
{
    LOG_TRA("");

    return true;
}

void NoneViewerAdapterImpl::render(const domain::model::Pose & /* pose */, const domain::model::ImagePacket & /* frame */)
{
    LOG_TRA("");
}
} // namespace vp::adapter::out