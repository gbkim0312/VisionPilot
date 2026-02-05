#include "vision_pilot_service_impl.hpp"
#include "gaia_log.hpp"
#include "vision_pilot_service.hpp"

namespace vp::service
{

VisionPilotServiceImpl::VisionPilotServiceImpl(vp::port::out::LocalizationPort &localization_port,
                                               vp::port::out::VisualizationPort &visualization_port,
                                               vp::port::out::ObjectDetectionPort &object_detection_port)
    : localization_port_{localization_port},
      visualization_port_{visualization_port},
      object_detection_port_{object_detection_port}
{
    LOG_TRA("Starting VisionPilot Service...");
}

VisionPilotServiceImpl::~VisionPilotServiceImpl()
{
    LOG_TRA("Stopping VisionPilot Service...");
}

void VisionPilotServiceImpl::onFrameReceived(const domain::model::ImagePacket &frame)
{
    auto pose = localization_port_.update(frame, frame.timestamp);
    auto detection = object_detection_port_.detectObject(frame);

    visualization_port_.render(pose, detection, frame);
}

} // namespace vp::service