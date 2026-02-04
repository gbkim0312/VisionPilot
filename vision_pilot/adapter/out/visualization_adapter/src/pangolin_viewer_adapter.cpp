#include "pangolin_viewer_adapter.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "pangolin_viewer_adapter_impl.hpp"
#include "viewer_config.hpp"

namespace vp::adapter::out
{
PangolinViewerAdapter::PangolinViewerAdapter(const config::ViewerConfig &config)
{
    LOG_TRA("");
    if (config.viewerType != config::ViewerType::PANGOLIN)
    {
        THROWLOG(SysException, "Type mismatch: PangolinViewerAdapter can be used only with PANGOLIN viewer type.");
    }
    impl_ = std::make_unique<PangolinViewerAdapterImpl>(config);
}
PangolinViewerAdapter::~PangolinViewerAdapter()
{
    LOG_TRA("");
}
bool PangolinViewerAdapter::start()
{
    return impl_->start();
}
bool PangolinViewerAdapter::stop()
{
    return impl_->stop();
}
void PangolinViewerAdapter::render(const domain::model::Pose &pose, const std::vector<domain::model::Detection> &detections, const domain::model::ImagePacket &frame)
{
    impl_->render(pose, detections, frame);
}
} // namespace vp::adapter::out