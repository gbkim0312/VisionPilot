#include "vision_pilot_service.hpp"
#include "gaia_log.hpp"
#include "localization_port.hpp"
#include "object_detection_port.hpp"
#include "object_tracking_port.hpp"
#include "vision_pilot_service_impl.hpp"

namespace vp::service
{
VisionPilotService::VisionPilotService(vp::port::out::LocalizationPort &localization_port, vp::port::out::VisualizationPort &visualization_port, vp::port::out::ObjectDetectionPort &object_detection_port, vp::port::out::ObjectTrackingPort &object_tracking_port)
    : impl_(std::make_unique<VisionPilotServiceImpl>(localization_port, visualization_port, object_detection_port, object_tracking_port))
{
    LOG_TRA("");
}

VisionPilotService::~VisionPilotService()
{
    LOG_TRA("");
}

void VisionPilotService::onFrameReceived(const domain::model::ImagePacket &frame)
{
    impl_->onFrameReceived(frame);
}
} // namespace vp::service