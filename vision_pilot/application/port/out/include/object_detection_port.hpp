#pragma once

#include "detection.hpp"
#include "image.hpp"

namespace vp::port::out
{
class ObjectDetectionPort
{
public:
    virtual ~ObjectDetectionPort() = default;
    virtual domain::model::DetectionResult detectObject(const vp::domain::model::ImagePacket &image) = 0;
};
} // namespace vp::port::out
