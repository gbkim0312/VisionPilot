#pragma once

#include "object_detection_port.hpp"

namespace vp::adapter::out
{
class NoDetectionAdapter : public port::out::ObjectDetectionPort
{
public:
    NoDetectionAdapter();

    ~NoDetectionAdapter() = default;

    bool initialize();
    domain::model::DetectionResult detectObject(const domain::model::ImagePacket &image) override;
};
} // namespace vp::adapter::out