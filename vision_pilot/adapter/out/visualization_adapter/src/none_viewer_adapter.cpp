#include "none_viewer_adapter.hpp"
#include "detection.hpp"
#include "gaia_log.hpp"
#include <gaia_exception.hpp>

namespace vp::adapter::out
{
NoneViewerAdapter::NoneViewerAdapter(const config::VslamViewerConfig &config)
{
    LOG_TRA("");

    if (config.viewerType != config::VslamViewerType::NONE)
    {
        THROWLOG(SysException, "NoneViewerAdapter created with non-NONE viewer type in config.");
    }
}
NoneViewerAdapter::~NoneViewerAdapter()
{
    LOG_TRA("");
}

bool NoneViewerAdapter::start()
{
    LOG_TRA("");
    return true;
}
bool NoneViewerAdapter::stop()
{
    LOG_TRA("");
    return true;
}
void NoneViewerAdapter::render(const domain::model::Pose & /* pose */, std::vector<domain::model::Detection> /* detections */, const domain::model::ImagePacket & /* frame */)
{
    LOG_TRA("");
    return;
}
} // namespace vp::adapter::out