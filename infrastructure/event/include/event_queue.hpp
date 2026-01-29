// infrastructure/event/include/event_queue.hpp
#pragma once
#include "event.hpp" // domain/model/event.hpp 전제
#include <condition_variable>
#include <mutex>
#include <queue>

namespace vp::infrastructure::event
{

class EventQueue
{
public:
    explicit EventQueue(size_t max_size = 10) : max_size_(max_size) {}

    void push(domain::model::Event event)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 큐가 가득 찼을 경우 가장 오래된 데이터를 버림 (자율주행 실시간성 유지)
        if (queue_.size() >= max_size_)
        {
            queue_.pop();
        }
        queue_.push(std::move(event));
        cv_.notify_one();
    }

    domain::model::Event pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]
                 { return !queue_.empty(); });

        domain::model::Event event = std::move(queue_.front());
        queue_.pop();
        return event;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<domain::model::Event> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    size_t max_size_;
};

} // namespace vp::infrastructure::event