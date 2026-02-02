#include "assembly_impl.hpp"
#include "gaia_log.hpp"
#include "in_adapter_registry.hpp"
#include "out_adapter_registry.hpp"
#include "vision_pilot_service.hpp"

namespace vp::assembly
{
AssemblyImpl::AssemblyImpl(const config::AssemblyConfig &config)
    : config_{config},
      event_queue_{}
{
    LOG_TRA("");
    out_adapter_registry_ = std::make_unique<OutAdapterRegistry>(config_);
    vision_pilot_service_ = std::make_unique<service::VisionPilotService>(
        out_adapter_registry_->getLocalizationPort(),
        out_adapter_registry_->getVisualizationPort(),
        out_adapter_registry_->getObjectDetectionPort());

    in_adapter_registry_ = std::make_unique<InAdapterRegistry>(config_, event_queue_, *vision_pilot_service_);

    event_router_ = std::make_unique<infrastructure::event::EventRouter>(event_queue_, in_adapter_registry_->getFrameReceiveUseCase());
}

AssemblyImpl::~AssemblyImpl()
{
    LOG_TRA("");
}

void AssemblyImpl::startService()
{
    LOG_TRA("");
    event_router_->start();
    in_adapter_registry_->startExternalAdapters();
    out_adapter_registry_->startExternalAdapters();
}

void AssemblyImpl::stopService()
{
    LOG_TRA("");
    event_router_->stop();
    in_adapter_registry_->stopExternalAdapters();
    out_adapter_registry_->stopExternalAdapters();
}

} // namespace vp::assembly