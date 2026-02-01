#pragma once

#include "detection.hpp"
#include "image.hpp"
#include <vector>

namespace vp::port::out
{
class ObjectDetectionPort
{
public:
    virtual ~ObjectDetectionPort() = default;
    virtual std::vector<vp::domain::model::Detection> detectObject(const vp::domain::model::ImagePacket &image) = 0;
};
} // namespace vp::port::out
