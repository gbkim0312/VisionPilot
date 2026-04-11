#pragma once
#include "detection.hpp"
#include "image.hpp"
#include "pose.hpp"
#include "tracking.hpp"

namespace vp::port::out
{

class VisualizationPort
{
public:
    virtual ~VisualizationPort() = default;

    virtual void render(const domain::model::Pose &pose, const domain::model::DetectionResult &detections, const domain::model::TrackingResult &tracking, const domain::model::ImagePacket &frame) = 0;
};

} // namespace vp::port::out