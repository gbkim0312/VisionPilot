#include "none_viewer_adapter.hpp"
#include "detection.hpp"
#include "gaia_log.hpp"
#include "viewer_config.hpp"
#include <gaia_exception.hpp>

namespace vp::adapter::out
{
NoneViewerAdapter::NoneViewerAdapter(const config::ViewerConfig &config)
    : config_(config)
{
    LOG_TRA("");
}
NoneViewerAdapter::~NoneViewerAdapter()
{
    LOG_TRA("");
}

bool NoneViewerAdapter::start() const
{
    LOG_TRA("");

    if (config_.viewerType != config::ViewerType::NONE)
    {
        LOG_WRN("Viewer type is not NONE, but NoneViewerAdapter is used.");
    }
    return true;
}
bool NoneViewerAdapter::stop() const
{
    LOG_TRA("");
    return true;
}
void NoneViewerAdapter::render(const domain::model::Pose &pose, const domain::model::DetectionResult &detections, const domain::model::ImagePacket &frame)
{
    LOG_TRA("");

    LOG_TRA("Received frame for rendering in None Viewer. Pose is_lost: {}, Detections count: {}, Frame id: {}", pose.is_lost, detections.detections.size(), frame.frame_id);
    LOG_TRA("X: {}, Y: {}, Z: {}", pose.x, pose.y, pose.z);

    std::string detection_info;
    for (const auto &detection : detections.detections)
    {
        detection_info +=
            "Class: " + domain::model::ClassIdHelper::toString(detection.class_id) +
            ", Confidence: " + std::to_string(detection.confidence) +
            ", BBox: [" + std::to_string(detection.bbox.x) + ", " +
            std::to_string(detection.bbox.y) + ", " +
            std::to_string(detection.bbox.width) + ", " +
            std::to_string(detection.bbox.height) + "]\n";
    }

    LOG_TRA("Detected: \n{}", detection_info);

    return;
}
} // namespace vp::adapter::out