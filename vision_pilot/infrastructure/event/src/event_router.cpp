// infrastructure/event/src/event_router.cpp
#include "event_router.hpp"
#include "gaia_log.hpp"
#include <exception>
#include <variant>

namespace vp::infrastructure::event
{

EventRouter::EventRouter(EventQueue &queue,
                         port::in::FrameReceiveUseCase &image_port)
    : queue_{queue}, image_port_{image_port}
{
    LOG_TRA("EventRouter initialized.");
}

EventRouter::~EventRouter()
{
    LOG_TRA("Shutting down EventRouter...");
    this->stop();
}

void EventRouter::start()
{
    LOG_INF("Starting EventRouter...");
    if (!running_)
    {
        running_ = true;
        worker_thread_ = std::thread(&EventRouter::run, this);
    }
}

void EventRouter::stop()
{
    LOG_INF("Stopping EventRouter...");
    running_ = false;
    if (worker_thread_.joinable())
    {
        worker_thread_.join();
    }
}

void EventRouter::run()
{
    LOG_TRA("EventRouter run loop started.");
    while (running_)
    {
        // 큐에서 이벤트 하나 꺼내기 (데이터가 올 때까지 blocking)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (queue_.empty())
        {
            continue;
        }
        domain::model::Event evt = queue_.pop();

        try
        {
            switch (evt.type)
            {
            case domain::model::EventType::IMAGE:
            {
                auto *packet = std::get_if<domain::model::ImageEventPayload>(&evt.data);
                if (packet != nullptr)
                {
                    image_port_.onFrameReceived(**packet);
                }
                break;
            }
            case domain::model::EventType::IMU:
                // imu_port_.onImuReceived(std::any_cast<ImuPacket>(evt.data));
            default:
                LOG_WRN("Unknown event type received in EventRouter.");
                break;
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERR("Event data casting failed: {}", e.what());
        }
    }
}

} // namespace vp::infrastructure::event