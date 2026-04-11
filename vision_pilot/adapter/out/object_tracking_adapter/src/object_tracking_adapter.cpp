#include "object_tracking_adapter.hpp"
#include "gaia_log.hpp"
#include "object_tracking_adapter_impl.hpp"

namespace vp::adapter::out
{
ObjectTrackingAdapter::ObjectTrackingAdapter()
    : impl_(std::make_unique<ObjectTrackingAdapterImpl>())
{
    LOG_TRA("");
}

ObjectTrackingAdapter::~ObjectTrackingAdapter()
{
    LOG_TRA("");
}

domain::model::TrackingResult ObjectTrackingAdapter::update(
    const std::optional<domain::model::DetectionResult> &detection,
    const domain::model::Pose &ego_pose,
    uint64_t timestamp)
{
    return impl_->update(detection, ego_pose, timestamp);
}
} // namespace vp::adapter::out
