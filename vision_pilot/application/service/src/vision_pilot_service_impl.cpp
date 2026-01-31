#include "vision_pilot_service_impl.hpp"
#include "gaia_log.hpp"

namespace vp::service
{
VisionPilotServiceImpl::VisionPilotServiceImpl(port::out::LocalizationPort &localization_port, port::out::VisualizationPort &visualization_port)
    : localization_port_{localization_port}, visualization_port_{visualization_port}
{
    LOG_TRA("");
}

VisionPilotServiceImpl::~VisionPilotServiceImpl()
{
    LOG_TRA("");
}

void VisionPilotServiceImpl::onFrameReceived(const domain::model::ImagePacket &frame)
{
    LOG_TRA("");
    auto pose = localization_port_.update(frame, 0);
    LOG_INF("Received Frame Processed. Pose: x={}, y={}, z={}", pose.x, pose.y, pose.z);
    visualization_port_.render(pose, frame);
}

} // namespace vp::service