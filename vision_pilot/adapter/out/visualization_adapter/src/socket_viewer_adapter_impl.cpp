#include "socket_viewer_adapter_impl.hpp"
#include "gaia_log.hpp"
#include "socket_viewer_adapter.hpp"

namespace vp::adapter::out
{
SocketViewerAdapterImpl::SocketViewerAdapterImpl(const config::ViewerConfig &config)
    : config_(config)
{
}

SocketViewerAdapterImpl::~SocketViewerAdapterImpl()
{
    LOG_TRA("");
}

bool SocketViewerAdapterImpl::start() const
{
    LOG_TRA("");

    return true;
}

bool SocketViewerAdapterImpl::stop() const
{
    LOG_TRA("");

    return true;
}

void SocketViewerAdapterImpl::render(const domain::model::Pose & /* pose */, const domain::model::DetectionResult & /* detections */, const domain::model::TrackingResult & /* tracking */, const domain::model::ImagePacket & /* frame */)
{
    LOG_TRA("");
}
} // namespace vp::adapter::out
