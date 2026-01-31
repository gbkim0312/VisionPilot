#pragma once
#include "localization_port.hpp"
#include "vision_pilot_service.hpp"

namespace vp::service
{
class VisionPilotServiceImpl
{
public:
    VisionPilotServiceImpl(port::out::LocalizationPort &localization_port, port::out::VisualizationPort &visualization_port);
    ~VisionPilotServiceImpl();

    void onFrameReceived(const domain::model::ImagePacket &frame);

private:
    port::out::LocalizationPort &localization_port_;
    port::out::VisualizationPort &visualization_port_;
    uint32_t _ = 0;
};
} // namespace vp::service