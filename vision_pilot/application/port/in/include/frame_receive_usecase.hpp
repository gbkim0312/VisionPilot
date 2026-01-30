#pragma once
#include "image.hpp"

namespace vp::port::in
{
class FrameReceiveUseCase
{
public:
    virtual ~FrameReceiveUseCase() = default;
    virtual void onFrameReceived(const domain::model::ImagePacket &frame) = 0;
};
} // namespace vp::port::in