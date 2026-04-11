#pragma once

#include "image.hpp"
#include "viewer_config.hpp"
#include "visualization_port.hpp"

namespace vp::adapter::out
{
class PangolinViewerAdapterImpl;

class PangolinViewerAdapter : public port::out::VisualizationPort
{
public:
    PangolinViewerAdapter(const config::ViewerConfig &config);
    ~PangolinViewerAdapter();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, const domain::model::DetectionResult &detections, const domain::model::TrackingResult &tracking, const domain::model::ImagePacket &frame) override;

private:
    std::unique_ptr<PangolinViewerAdapterImpl> impl_;
};
} // namespace vp::adapter::out