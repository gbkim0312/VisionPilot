#pragma once
#include "image.hpp"
#include "pose.hpp"

namespace vp::port::out
{

class VisualizationPort
{
public:
    virtual ~VisualizationPort() = default;

    virtual void render(const domain::model::Pose &pose, const domain::model::ImagePacket &frame) = 0;
};

} // namespace vp::port::out