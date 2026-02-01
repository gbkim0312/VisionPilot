#pragma once

#include "image.hpp"
#include "visualization_port.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{
class PangolinViewerAdapterImpl;

class PangolinViewerAdapter : public port::out::VisualizationPort
{
public:
    PangolinViewerAdapter(const config::VslamViewerConfig &config);
    ~PangolinViewerAdapter();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, std::vector<domain::model::Detection> detections, const domain::model::ImagePacket &frame) override;

private:
    std::unique_ptr<PangolinViewerAdapterImpl> impl_;
};
} // namespace vp::adapter::out