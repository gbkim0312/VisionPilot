#include "no_detection_adapter.hpp"
#include "gaia_log.hpp"

namespace vp::adapter::out
{
NoDetectionAdapter::NoDetectionAdapter()
{
    LOG_TRA("");
}

bool NoDetectionAdapter::initialize()
{
    LOG_INF("NoDetectionAdapter initialized. This adapter does not perform any object detection.");
    return true;
}

std::vector<domain::model::Detection> NoDetectionAdapter::detectObject(const domain::model::ImagePacket & /* image */)
{
    return {};
}

} // namespace vp::adapter::out