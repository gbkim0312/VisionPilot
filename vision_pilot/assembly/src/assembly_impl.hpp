#pragma once

#include "assembly_config.hpp"
#include "event_queue.hpp"
#include "event_router.hpp"
#include "frame_receive_usecase.hpp"
#include "in_adapter_registry.hpp"
#include "out_adapter_registry.hpp"
#include "vision_pilot_service.hpp"
#include <memory>

namespace vp::assembly
{
class AssemblyImpl
{
public:
    AssemblyImpl(const config::AssemblyConfig &config);
    ~AssemblyImpl();

    void startService();
    void stopService();

private:
    const config::AssemblyConfig &config_;
    infrastructure::event::EventQueue event_queue_;

    std::unique_ptr<port::in::FrameReceiveUseCase> frame_receive_use_case_;
    std::unique_ptr<OutAdapterRegistry> out_adapter_registry_;
    std::unique_ptr<InAdapterRegistry> in_adapter_registry_;
    std::unique_ptr<service::VisionPilotService> vision_pilot_service_;
    std::unique_ptr<infrastructure::event::EventRouter> event_router_;
};
} // namespace vp::assembly