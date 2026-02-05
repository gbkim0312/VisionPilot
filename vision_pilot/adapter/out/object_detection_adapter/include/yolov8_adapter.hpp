#pragma once

#include "detection.hpp"
#include "detection_config.hpp"
#include "object_detection_port.hpp"
#include <memory>
#include <vector>

namespace vp::adapter::out
{
class YOLOv8AdapterImpl;

class YOLOv8Adapter : public vp::port::out::ObjectDetectionPort
{
public:
    YOLOv8Adapter(const config::DetectionConfig &config);
    ~YOLOv8Adapter() override;

    bool start();
    std::vector<vp::domain::model::Detection> detectObject(const vp::domain::model::ImagePacket &image) override;
    bool stop();

private:
    std::unique_ptr<YOLOv8AdapterImpl> impl_;
};
} // namespace vp::adapter::out