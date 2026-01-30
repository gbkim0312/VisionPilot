#include "vision_pilot_service_impl.hpp"
#include "gaia_log.hpp"

namespace vp::service
{
VisionPilotServiceImpl::VisionPilotServiceImpl()
{
    LOG_TRA("");
}

VisionPilotServiceImpl::~VisionPilotServiceImpl()
{
    LOG_TRA("");
}

void VisionPilotServiceImpl::onFrameReceived(const domain::model::ImagePacket & /* frame */)
{
    LOG_TRA("");
    (void)_;
}

} // namespace vp::service