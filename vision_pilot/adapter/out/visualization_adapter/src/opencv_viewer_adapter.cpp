#include "opencv_viewer_adapter.hpp"
#include "gaia_log.hpp"
#include "opencv_viewer_adapter_impl.hpp"

namespace vp::adapter::out
{
OpenCVViewerAdapter::OpenCVViewerAdapter(const config::VslamViewerConfig &config)
    : impl_(std::make_unique<OpenCVViewerAdapterImpl>(config))
{
}

OpenCVViewerAdapter::~OpenCVViewerAdapter()
{
    LOG_TRA("");
}

bool OpenCVViewerAdapter::start()
{
    return impl_->start();
}
bool OpenCVViewerAdapter::stop()
{
    return impl_->stop();
}

void OpenCVViewerAdapter::render(const domain::model::Pose &pose, const domain::model::ImagePacket &frame)
{
    impl_->render(pose, frame);
}
} // namespace vp::adapter::out