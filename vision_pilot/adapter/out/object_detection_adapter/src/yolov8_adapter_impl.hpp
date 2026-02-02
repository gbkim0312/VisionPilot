#pragma once

#include "detection.hpp"
#include "detection_config.hpp"
#include "image.hpp"
#include <memory>
#include <opencv2/dnn.hpp>
#include <vector>

namespace vp::adapter::out
{
class YOLOv8AdapterImpl
{
public:
    YOLOv8AdapterImpl(const config::YoloConfig &config);
    ~YOLOv8AdapterImpl();

    bool initialize();
    std::vector<vp::domain::model::Detection> detectObject(const vp::domain::model::ImagePacket &image);
    bool deinitialize();

private:
    std::mutex mutex_;
    bool is_initialized_ = false;
    std::unique_ptr<cv::dnn::Net> net_;
    const config::YoloConfig &config_;
};
} // namespace vp::adapter::out
