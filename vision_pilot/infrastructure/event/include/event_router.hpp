// infrastructure/event/include/event_router.hpp
#pragma once

#include "event_queue.hpp"
#include "frame_receive_usecase.hpp"
#include <atomic>
#include <thread>

namespace vp::infrastructure::event
{

class EventRouter
{
public:
    EventRouter(EventQueue &queue, port::in::FrameReceiveUseCase &image_port);

    ~EventRouter();

    void start();
    void stop();

private:
    void run();

    // 참조 멤버 변수
    EventQueue &queue_;
    port::in::FrameReceiveUseCase &image_port_;

    std::thread worker_thread_;
    std::atomic<bool> running_{false};
};

} // namespace vp::infrastructure::event