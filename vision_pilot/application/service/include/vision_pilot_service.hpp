#pragma once
#include "frame_receive_usecase.hpp"
#include "localization_port.hpp"
#include "object_detection_port.hpp"
#include "visualization_port.hpp"
#include <memory>

namespace vp::service
{
class VisionPilotServiceImpl;

class VisionPilotService : public vp::port::in::FrameReceiveUseCase
{
public:
    VisionPilotService(vp::port::out::LocalizationPort &localization_port, vp::port::out::VisualizationPort &visualization_port, vp::port::out::ObjectDetectionPort &object_detection_port);
    ~VisionPilotService();

    void onFrameReceived(const domain::model::ImagePacket &frame) override;

private:
    std::unique_ptr<VisionPilotServiceImpl> impl_;
};
} // namespace vp::service