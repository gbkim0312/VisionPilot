#include "none_viewer_adapter.hpp"
#include "detection.hpp"
#include "gaia_log.hpp"
#include <gaia_exception.hpp>

namespace vp::adapter::out
{
NoneViewerAdapter::NoneViewerAdapter(const config::VslamViewerConfig &config)
    : config_(config)
{
    LOG_TRA("");
}
NoneViewerAdapter::~NoneViewerAdapter()
{
    LOG_TRA("");
}

bool NoneViewerAdapter::start() const
{
    LOG_TRA("");

    if (config_.viewerType != config::VslamViewerType::NONE)
    {
        LOG_WRN("Viewer type is not NONE, but NoneViewerAdapter is used.");
    }
    return true;
}
bool NoneViewerAdapter::stop() const
{
    LOG_TRA("");
    return true;
}
void NoneViewerAdapter::render(const domain::model::Pose & /* pose */, const std::vector<domain::model::Detection> & /* detections */, const domain::model::ImagePacket & /* frame */)
{
    LOG_TRA("");
    return;
}
} // namespace vp::adapter::out