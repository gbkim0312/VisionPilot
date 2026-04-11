#pragma once

#include "image.hpp"
#include "viewer_config.hpp"
#include "visualization_port.hpp"

namespace vp::adapter::out
{
class SocketViewerAdapterImpl;

class SocketViewerAdapter : public port::out::VisualizationPort
{
public:
    SocketViewerAdapter(const config::ViewerConfig &config);
    ~SocketViewerAdapter();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, const domain::model::DetectionResult &detections, const domain::model::TrackingResult &tracking, const domain::model::ImagePacket &frame) override;

private:
    std::unique_ptr<SocketViewerAdapterImpl> impl_;
};
} // namespace vp::adapter::out