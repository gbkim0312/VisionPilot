#include "socket_viewer_adapter_impl.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "socket_viewer_adapter.hpp"

namespace vp::adapter::out
{
SocketViewerAdapterImpl::SocketViewerAdapterImpl(const config::VslamViewerConfig &config)
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

    if (config_.viewerType != config::VslamViewerType::SOCKET)
    {
        THROWLOG(SysException, "Type mismatch: SocketViewerAdapterImpl can be used only with SOCKET viewer type.");
    }
    return true;
}

bool SocketViewerAdapterImpl::stop() const
{
    LOG_TRA("");
    return true;
}

void SocketViewerAdapterImpl::render(const domain::model::Pose & /* pose */, const std::vector<domain::model::Detection> & /* detections */, const domain::model::ImagePacket & /* frame */)
{
    LOG_TRA("");
}
} // namespace vp::adapter::out
