#pragma once
#include "vision_pilot_service.hpp"

namespace vp::service
{
class VisionPilotServiceImpl
{
public:
    VisionPilotServiceImpl();
    ~VisionPilotServiceImpl();

    void onFrameReceived(const domain::model::ImagePacket &frame);

private:
    uint32_t _ = 0;
};
} // namespace vp::service