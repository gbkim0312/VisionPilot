#pragma once
#include "event_queue.hpp" // 추가
#include "video_loader.hpp"
#include "video_loader_config.hpp"
#include <atomic>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <thread>

namespace vp::adapter::in::frame_loader
{
class VideoLoaderImpl
{
public:
    VideoLoaderImpl(const config::VideoLoaderConfig &config, infrastructure::event::EventQueue &event_queue);
    ~VideoLoaderImpl();

    bool start();
    bool stop();

private:
    void loadFrames();

    const config::VideoLoaderConfig &config_;
    std::atomic_bool running_ = false;
    std::thread worker_thread_;

    infrastructure::event::EventQueue &event_queue_; // 포트 대신 큐 참조
    std::unique_ptr<cv::VideoCapture> video_capture_;
};
} // namespace vp::adapter::in::frame_loader