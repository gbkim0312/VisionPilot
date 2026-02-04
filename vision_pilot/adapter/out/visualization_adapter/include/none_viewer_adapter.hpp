#pragma once

#include "image.hpp"
#include "viewer_config.hpp"
#include "visualization_port.hpp"

namespace vp::adapter::out
{
class NoneViewerAdapter : public port::out::VisualizationPort
{
public:
    NoneViewerAdapter(const config::ViewerConfig &config);
    ~NoneViewerAdapter();
    bool start() const;
    bool stop() const;

    void render(const domain::model::Pose &pose, const std::vector<domain::model::Detection> &detections, const domain::model::ImagePacket &frame) override;

private:
    const config::ViewerConfig &config_;
};
} // namespace vp::adapter::out