#include "socket_viewer_adapter_impl.hpp"
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

bool SocketViewerAdapterImpl::start()
{
    LOG_TRA("");
    return true;
}

bool SocketViewerAdapterImpl::stop()
{
    LOG_TRA("");
    return true;
}

void SocketViewerAdapterImpl::render(const domain::model::Pose & /* pose */, const domain::model::ImagePacket & /* frame */)
{
    LOG_TRA("");
}
} // namespace vp::adapter::out
