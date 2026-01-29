#pragma once
#include "video_loader_config.hpp"
#include <memory>

namespace vp::infrastructure::event
{
class EventQueue;
} // namespace vp::infrastructure::event

namespace vp::adapter::in::frame_loader
{
class VideoLoaderImpl;

class VideoLoader
{
public:
    // 포트 대신 큐를 참조로 받음
    VideoLoader(const config::VideoLoaderConfig &config, infrastructure::event::EventQueue &event_queue);
    ~VideoLoader();

    bool start();
    bool stop();

private:
    std::unique_ptr<VideoLoaderImpl> impl_;
};
} // namespace vp::adapter::in::frame_loader