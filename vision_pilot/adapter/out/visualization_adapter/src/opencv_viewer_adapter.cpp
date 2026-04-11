#include "opencv_viewer_adapter.hpp"
#include "gaia_exception.hpp"
#include "gaia_log.hpp"
#include "opencv_viewer_adapter_impl.hpp"
#include "viewer_config.hpp"

namespace vp::adapter::out
{
OpenCVViewerAdapter::OpenCVViewerAdapter(const config::ViewerConfig &config)
{
    LOG_TRA("");
    if (config.viewerType != config::ViewerType::OPENCV)
    {
        THROWLOG(SysException, "Type mismatch: OpenCVViewerAdapter can be used only with OPENCV viewer type.");
    }
    impl_ = std::make_unique<OpenCVViewerAdapterImpl>(config);
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

void OpenCVViewerAdapter::render(const domain::model::Pose &pose, const domain::model::DetectionResult &detections, const domain::model::TrackingResult &tracking, const domain::model::ImagePacket &frame)
{
    impl_->render(pose, detections, tracking, frame);
}
} // namespace vp::adapter::out