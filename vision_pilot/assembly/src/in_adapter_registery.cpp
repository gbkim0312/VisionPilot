#include "frame_receive_usecase.hpp"
#include "gaia_log.hpp"
#include "in_adapter_registry.hpp"
#include "video_loader_factory.hpp"
#include "vision_pilot_service.hpp"
namespace vp::assembly
{
InAdapterRegistry::InAdapterRegistry(const config::AssemblyConfig &config,
                                     infrastructure::event::EventQueue &event_queue,
                                     service::VisionPilotService &vision_pilot_service)
    : config_(config),
      event_queue_(event_queue),
      vision_pilot_service_{vision_pilot_service}
{
    LOG_TRA("");

    video_loader_ = vp::adapter::in::VideoLoaderFactory::createVideoLoader(config_.videoLoaderConfig, event_queue_);
}

InAdapterRegistry::~InAdapterRegistry()
{
    LOG_TRA("");
}

void InAdapterRegistry::startExternalAdapters()
{
    LOG_TRA("");
    video_loader_->start();
}

void InAdapterRegistry::stopExternalAdapters()
{
    LOG_TRA("");
    video_loader_->stop();
}

port::in::FrameReceiveUseCase &InAdapterRegistry::getFrameReceiveUseCase()
{
    LOG_TRA("");
    return vision_pilot_service_;
}
} // namespace vp::assembly