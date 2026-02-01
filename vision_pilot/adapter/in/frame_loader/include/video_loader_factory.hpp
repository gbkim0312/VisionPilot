#pragma once
#include "event_queue.hpp"
#include "video_loader.hpp"
#include "video_loader_config.hpp"
#include <memory>

namespace vp::adapter::in
{
class VideoLoaderFactory
{
public:
    static std::unique_ptr<VideoLoader> createVideoLoader(const config::VideoLoaderConfig &config, infrastructure::event::EventQueue &event_queue);
};
} // namespace vp::adapter::in