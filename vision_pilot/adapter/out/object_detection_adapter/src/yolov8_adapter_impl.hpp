#pragma once

#include "detection.hpp"
#include "detection_config.hpp"
#include "gaia_circular_queue.hpp"
#include "image.hpp"
#include <memory>
#include <opencv2/dnn.hpp>
#include <thread>
#include <vector>
namespace vp::adapter::out
{
class YOLOv8AdapterImpl
{
public:
    YOLOv8AdapterImpl(const config::YoloConfig &config);
    ~YOLOv8AdapterImpl();

    bool start();
    std::vector<vp::domain::model::Detection> detectObject(const vp::domain::model::ImagePacket &image);
    bool stop();

private:
    std::mutex mutex_;
    std::atomic_bool is_running_ = false;
    std::unique_ptr<cv::dnn::Net> net_;
    const config::YoloConfig &config_;

    std::vector<domain::model::Detection> last_detections_;
    std::thread detection_thread_;
    ThreadSafeCircularQueue<domain::model::ImagePacket> image_queue_;

    std::vector<vp::domain::model::Detection> detectObjectImpl(const vp::domain::model::ImagePacket &image);
    void runDetection();
};
} // namespace vp::adapter::out
