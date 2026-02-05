#pragma once

#include "detection.hpp"
#include "detection_config.hpp"
#include "gaia_circular_queue.hpp"
#include "image.hpp"
#include <memory>
#include <opencv2/dnn.hpp>
#include <thread>

namespace vp::adapter::out
{
class YOLOv8AdapterImpl
{
public:
    YOLOv8AdapterImpl(const config::YoloConfig &config);
    ~YOLOv8AdapterImpl();

    bool start();
    domain::model::DetectionResult detectObject(const vp::domain::model::ImagePacket &image);
    bool stop();

private:
    std::mutex mutex_;
    std::atomic_bool is_running_ = false;
    std::unique_ptr<cv::dnn::Net> net_;
    const config::YoloConfig &config_;

    domain::model::DetectionResult last_detections_;
    std::thread detection_thread_;
    ThreadSafeCircularQueue<domain::model::ImagePacket> image_queue_;

    domain::model::DetectionResult detectObjectImpl(const vp::domain::model::ImagePacket &image);
    void runDetection();
};
} // namespace vp::adapter::out
