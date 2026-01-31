#pragma once

#include "image.hpp"
#include "visualization_port.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{
class SocketViewerAdapterImpl;

class SocketViewerAdapter : public port::out::VisualizationPort
{
public:
    SocketViewerAdapter(const config::VslamViewerConfig &config);
    ~SocketViewerAdapter();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, const domain::model::ImagePacket &frame) override;

private:
    std::unique_ptr<SocketViewerAdapterImpl> impl_;
};
} // namespace vp::adapter::out