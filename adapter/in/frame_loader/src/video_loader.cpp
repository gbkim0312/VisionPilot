#include "video_loader.hpp"
#include "event_queue.hpp"
#include "gaia_log.hpp"
#include "video_loader_impl.hpp"
namespace vp::adapter::in::frame_loader
{
VideoLoader::VideoLoader(const config::VideoLoaderConfig &config, infrastructure::event::EventQueue &event_queue)
    : impl_(std::make_unique<VideoLoaderImpl>(config, event_queue))
{
    LOG_TRA("");
}

VideoLoader::~VideoLoader() = default;

bool VideoLoader::start()
{
    return impl_->start();
}

bool VideoLoader::stop()
{
    return impl_->stop();
}
} // namespace vp::adapter::in::frame_loader