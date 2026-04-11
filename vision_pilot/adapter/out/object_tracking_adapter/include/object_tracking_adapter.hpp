#pragma once

#include "object_tracking_port.hpp"
#include <memory>

namespace vp::adapter::out
{
class ObjectTrackingAdapterImpl;

class ObjectTrackingAdapter : public port::out::ObjectTrackingPort
{
public:
    ObjectTrackingAdapter();
    ~ObjectTrackingAdapter();

    domain::model::TrackingResult update(
        const std::optional<domain::model::DetectionResult> &detection,
        const domain::model::Pose &ego_pose,
        uint64_t timestamp) override;

private:
    std::unique_ptr<ObjectTrackingAdapterImpl> impl_;
};
} // namespace vp::adapter::out