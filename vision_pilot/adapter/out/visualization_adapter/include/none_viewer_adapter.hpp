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
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, const domain::model::ImagePacket &frame) override;

private:
};
} // namespace vp::adapter::out