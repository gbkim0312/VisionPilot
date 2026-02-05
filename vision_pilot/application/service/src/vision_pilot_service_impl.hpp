#pragma once

#include "localization_port.hpp"
#include "object_detection_port.hpp"
#include "vision_pilot_service.hpp"
#include "visualization_port.hpp"

namespace vp::service
{

class VisionPilotServiceImpl
{
public:
    VisionPilotServiceImpl(vp::port::out::LocalizationPort &localization_port,
                           vp::port::out::VisualizationPort &visualization_port,
                           vp::port::out::ObjectDetectionPort &object_detection_port);
    ~VisionPilotServiceImpl();

    void onFrameReceived(const domain::model::ImagePacket &frame);

private:
    void detectionLoop();

private:
    vp::port::out::LocalizationPort &localization_port_;
    vp::port::out::VisualizationPort &visualization_port_;
    vp::port::out::ObjectDetectionPort &object_detection_port_;
};

} // namespace vp::service