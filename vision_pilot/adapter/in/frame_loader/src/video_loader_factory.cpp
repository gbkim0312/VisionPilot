#include "video_loader_factory.hpp"
#include "gaia_log.hpp"
#include "mono_video_loader.hpp"
#include "video_loader.hpp"

namespace vp::adapter::in
{
std::unique_ptr<VideoLoader> VideoLoaderFactory::createVideoLoader(const config::VideoLoaderConfig &config, infrastructure::event::EventQueue &event_queue)
{
    switch (config.cameraFormat)
    {
    case config::CameraFormat::MONO:
        return std::make_unique<MonoVideoLoader>(config, event_queue);
    default:
        LOG_ERR("Unsupported CameraFormat: {}", config::toString(config.cameraFormat));
        return nullptr;
    }
}
} // namespace vp::adapter::in