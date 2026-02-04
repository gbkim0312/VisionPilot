
#pragma once

#include "image.hpp"
#include "viewer_config.hpp"
#include "visualization_port.hpp"

namespace vp::adapter::out
{
class OpenCVViewerAdapterImpl;

class OpenCVViewerAdapter : public port::out::VisualizationPort
{
public:
    OpenCVViewerAdapter(const config::ViewerConfig &config);
    ~OpenCVViewerAdapter();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, const std::vector<domain::model::Detection> &detections, const domain::model::ImagePacket &frame) override;

private:
    std::unique_ptr<OpenCVViewerAdapterImpl> impl_;
};
} // namespace vp::adapter::out