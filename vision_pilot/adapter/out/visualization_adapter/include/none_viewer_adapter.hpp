#pragma once

#include "image.hpp"
#include "visualization_port.hpp"
#include "vslam_config.hpp"
#include <memory>

namespace vp::adapter::out
{
class NoneViewerAdapterImpl;

class NoneViewerAdapter : public port::out::VisualizationPort
{
public:
    NoneViewerAdapter(const config::VslamViewerConfig &config);
    ~NoneViewerAdapter();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, const domain::model::ImagePacket &frame) override;

private:
    std::unique_ptr<NoneViewerAdapterImpl> impl_;
};
} // namespace vp::adapter::out