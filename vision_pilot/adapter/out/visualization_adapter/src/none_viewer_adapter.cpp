#include "none_viewer_adapter.hpp"
#include "gaia_log.hpp"
#include "none_viewer_adapter_impl.hpp"

namespace vp::adapter::out
{
NoneViewerAdapter::NoneViewerAdapter(const config::VslamViewerConfig &config)
    : impl_(std::make_unique<NoneViewerAdapterImpl>(config))
{
    LOG_TRA("");
}
NoneViewerAdapter::~NoneViewerAdapter()
{
    LOG_TRA("");
}

bool NoneViewerAdapter::start()
{
    LOG_TRA("");
    return impl_->start();
}
bool NoneViewerAdapter::stop()
{
    LOG_TRA("");
    return impl_->stop();
}
void NoneViewerAdapter::render(const domain::model::Pose &pose, const domain::model::ImagePacket &frame)
{
    LOG_TRA("");
    impl_->render(pose, frame);
}
} // namespace vp::adapter::out