#include "pangolin_viewer_adapter.hpp"
#include "gaia_log.hpp"
#include "pangolin_viewer_adapter_impl.hpp"

namespace vp::adapter::out
{
PangolinViewerAdapter::PangolinViewerAdapter(const config::VslamViewerConfig &config)
    : impl_(std::make_unique<PangolinViewerAdapterImpl>(config))
{
    LOG_TRA("");
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
void PangolinViewerAdapter::render(const domain::model::Pose &pose, std::vector<domain::model::Detection> detections, const domain::model::ImagePacket &frame)
{
    impl_->render(pose, detections, frame);
}
} // namespace vp::adapter::out