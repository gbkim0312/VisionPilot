#pragma once
#include "assembly_config.hpp"
#include "event_queue.hpp"
#include "video_loader.hpp"
#include "vision_pilot_service.hpp"
namespace vp::assembly
{
class InAdapterRegistry
{
public:
    explicit InAdapterRegistry(const config::AssemblyConfig &config,
                               infrastructure::event::EventQueue &event_queue,
                               service::VisionPilotService &vision_pilot_service);
    ~InAdapterRegistry();

    void startExternalAdapters();
    void stopExternalAdapters();

    port::in::FrameReceiveUseCase &getFrameReceiveUseCase();

private:
    const config::AssemblyConfig &config_;

    infrastructure::event::EventQueue &event_queue_;
    std::unique_ptr<vp::adapter::in::VideoLoader> video_loader_;

    service::VisionPilotService &vision_pilot_service_;
};
} // namespace vp::assembly
