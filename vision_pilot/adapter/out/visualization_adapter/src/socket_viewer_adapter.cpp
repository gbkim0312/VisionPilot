#include "socket_viewer_adapter.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "socket_viewer_adapter_impl.hpp"

namespace vp::adapter::out
{
SocketViewerAdapter::SocketViewerAdapter(const config::VslamViewerConfig &config)
{
    LOG_TRA("");

    if (config.viewerType != config::VslamViewerType::SOCKET)
    {
        THROWLOG(SysException, "Type mismatch: SocketViewerAdapter can be used only with SOCKET viewer type.");
    }
    impl_ = std::make_unique<SocketViewerAdapterImpl>(config);
}

SocketViewerAdapter::~SocketViewerAdapter()
{
    LOG_TRA("");
}

bool SocketViewerAdapter::start()
{
    return impl_->start();
}

bool SocketViewerAdapter::stop()
{
    return impl_->stop();
}

void SocketViewerAdapter::render(const domain::model::Pose &pose, const std::vector<domain::model::Detection> &detections, const domain::model::ImagePacket &frame)
{
    impl_->render(pose, detections, frame);
}
} // namespace vp::adapter::out
