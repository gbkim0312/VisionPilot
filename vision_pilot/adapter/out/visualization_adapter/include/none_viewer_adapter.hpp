#pragma once

#include "image.hpp"
#include "visualization_port.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{
class NoneViewerAdapter : public port::out::VisualizationPort
{
public:
    NoneViewerAdapter(const config::VslamViewerConfig &config);
    ~NoneViewerAdapter();
    bool start() const;
    bool stop() const;

    void render(const domain::model::Pose &pose, const std::vector<domain::model::Detection> &detections, const domain::model::ImagePacket &frame) override;

private:
    config::VslamViewerConfig config_;
};
} // namespace vp::adapter::out