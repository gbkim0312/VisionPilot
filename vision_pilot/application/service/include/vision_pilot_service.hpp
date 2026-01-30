#pragma once
#include "frame_receive_usecase.hpp"
#include <memory>

namespace vp::service
{
class VisionPilotServiceImpl;

class VisionPilotService : public vp::port::in::FrameReceiveUseCase
{
public:
    VisionPilotService();
    ~VisionPilotService();

    void onFrameReceived(const domain::model::ImagePacket &frame) override;

private:
    std::unique_ptr<VisionPilotServiceImpl> impl_;
};
} // namespace vp::service