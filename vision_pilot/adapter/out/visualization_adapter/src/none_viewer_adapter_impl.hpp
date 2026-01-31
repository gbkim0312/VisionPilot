#pragma once

#include "image.hpp"
#include "pose.hpp"
#include "vslam_config.hpp"

namespace vp::adapter::out
{
class NoneViewerAdapterImpl
{
public:
    NoneViewerAdapterImpl(const config::VslamViewerConfig &config);
    ~NoneViewerAdapterImpl();
    bool start();
    bool stop();

    void render(const domain::model::Pose &pose, const domain::model::ImagePacket &frame);

private:
};
} // namespace vp::adapter::out