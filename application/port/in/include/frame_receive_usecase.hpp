#pragma once
#include "image.hpp"
#include <memory>

namespace vp::port::in
{
class FrameReceiveUseCase
{
public:
    virtual ~FrameReceiveUseCase() = default;
    virtual void onFrameReceived(std::shared_ptr<domain::model::ImagePacket> frame) = 0;
};
} // namespace vp::port::in