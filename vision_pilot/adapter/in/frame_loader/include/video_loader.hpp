#pragma once
#include "event_queue.hpp"
#include "image.hpp"
#include "video_loader_config.hpp"
#include <atomic>
#include <memory>
#include <thread>

namespace vp::adapter::in
{

class VideoLoader
{
public:
    VideoLoader(const config::VideoLoaderConfig &config, infrastructure::event::EventQueue &event_queue);
    virtual ~VideoLoader();

    bool start();
    bool stop();

protected:
    virtual bool initialize() = 0;
    virtual bool fetchFrame() = 0;
    virtual void release() = 0;

    void pushToQueue(const std::shared_ptr<domain::model::ImagePacket> &frame_packet);
    void runLoop();

    void loadFramesFromFile();
    void loadFramesFromCameraDevice();

protected:
    const config::VideoLoaderConfig &config_;
    infrastructure::event::EventQueue &event_queue_;

    std::atomic_bool running_ = false;
    std::thread worker_thread_;
    uint64_t frame_id_ = 0;
};
} // namespace vp::adapter::in