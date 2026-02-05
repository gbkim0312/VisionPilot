#pragma once

#include "gaia_log.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace
{
constexpr auto kTimeoueSeconds = 2;
}

namespace vp
{
// Thread-safe circular queue (ring buffer) with optional overwrite-on-full.
template <typename T>
class ThreadSafeCircularQueue
{
public:
    ThreadSafeCircularQueue(size_t capacity,
                            const std::string &queue_name = "circular queue",
                            bool overwrite_on_full = true);

    ~ThreadSafeCircularQueue();

    bool empty() const;
    int size() const;
    size_t capacity() const;

    // 즉시 enqueue
    // - 큐가 가득 찼을 때 overwrite_on_full_ 이면 가장 오래된 항목을 덮어씀
    void enqueue(const T &item);

    // 즉시 dequeue (없으면 false)
    bool dequeue(T &item);

    // 빈 슬롯을 기다렸다가 enqueue
    // - overwrite_on_full_ == true 면 기다리지 않고 즉시 덮어쓰고 true
    // - overwrite_on_full_ == false 면 빈 슬롯 생길 때까지 대기 (타임아웃 시 false)
    bool waitAndEnqueue(const T &item);

    // 항목이 생길 때까지 대기 후 dequeue (타임아웃 시 false)
    bool waitAndDeque(T &item);

private:
    void flush();

    // 내부 헬퍼
    size_t nextIndex(size_t idx) const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cond_not_empty_;
    std::condition_variable cond_not_full_;

    std::vector<T> buffer_;
    size_t capacity_;
    size_t head_; // oldest element index
    size_t tail_; // next write position
    size_t count_;

    bool overwrite_on_full_;
    uint32_t overwrite_count_ = 0;
    std::string queue_name_;
};

template <typename T>
ThreadSafeCircularQueue<T>::ThreadSafeCircularQueue(size_t capacity,
                                                    const std::string &queue_name,
                                                    bool overwrite_on_full)
    : buffer_(capacity ? capacity : 1),
      capacity_{capacity ? capacity : 1},
      head_{0},
      tail_{0},
      count_{0},
      overwrite_on_full_{overwrite_on_full},
      queue_name_{queue_name}
{
    LOG_INF("Creating circular queue. Capacity: {} | Name: {} | Overwrite: {}\n",
            capacity_, queue_name_, overwrite_on_full_);
}

template <typename T>
ThreadSafeCircularQueue<T>::~ThreadSafeCircularQueue()
{
    LOG_TRA("{}", queue_name_);
}

template <typename T>
bool ThreadSafeCircularQueue<T>::empty() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return count_ == 0;
}

template <typename T>
int ThreadSafeCircularQueue<T>::size() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return static_cast<int>(count_);
}

template <typename T>
size_t ThreadSafeCircularQueue<T>::capacity() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return capacity_;
}

template <typename T>
void ThreadSafeCircularQueue<T>::enqueue(const T &item)
{
    LOG_TRA("{}", queue_name_);

    std::unique_lock<std::mutex> lock(mutex_);

    if (count_ == capacity_)
    {
        if (overwrite_on_full_)
        {
            // 가장 오래된 요소를 버리고(head 전진) 덮어씀
            head_ = nextIndex(head_);
            ++overwrite_count_;
            LOG_DBG("Circular queue full. Overwriting oldest. ({}, overwritten={})",
                    queue_name_, overwrite_count_);
        }
        else
        {
            // 드롭하고 싶다면 여기서 return; (현재는 대기 대신 드롭하지 않고 실패 없이 무시하려면 정책 결정)
            // LOG_WRN("Circular queue full. Drop item. ({})", queue_name_);
            // return;

            // 혹은 flush() 정책으로 바꾸고 싶으면:
            // this->flush();
        }
    }
    else
    {
        ++count_;
    }

    buffer_[tail_] = item;
    tail_ = nextIndex(tail_);

    // 항목 생김
    cond_not_empty_.notify_one();
}

template <typename T>
bool ThreadSafeCircularQueue<T>::dequeue(T &item)
{
    LOG_TRA("{}", queue_name_);

    std::unique_lock<std::mutex> lock(mutex_);
    if (count_ == 0)
    {
        return false;
    }

    item = buffer_[head_];
    head_ = nextIndex(head_);
    --count_;

    // 빈 슬롯 생김
    cond_not_full_.notify_one();
    return true;
}

template <typename T>
bool ThreadSafeCircularQueue<T>::waitAndEnqueue(const T &item)
{
    LOG_TRA("{}", queue_name_);

    std::unique_lock<std::mutex> lock(mutex_);

    if (overwrite_on_full_)
    {
        // 기다리지 않고 즉시 overwrite
        if (count_ == capacity_)
        {
            head_ = nextIndex(head_);
            ++overwrite_count_;
            LOG_DBG("Circular queue full (wait). Overwriting oldest. ({}, overwritten={})",
                    queue_name_, overwrite_count_);
        }
        else
        {
            ++count_;
        }

        buffer_[tail_] = item;
        tail_ = nextIndex(tail_);
        cond_not_empty_.notify_one();
        return true;
    }

    // overwrite 모드가 아니면 빈 슬롯 생길 때까지 대기
    if (!cond_not_full_.wait_for(lock, std::chrono::seconds(kTimeoueSeconds), [this]
                                 { return count_ < capacity_; }))
    {
        LOG_WRN("Timed out while waiting for space. ({})", queue_name_);
        return false;
    }

    ++count_;
    buffer_[tail_] = item;
    tail_ = nextIndex(tail_);
    cond_not_empty_.notify_one();
    return true;
}

template <typename T>
bool ThreadSafeCircularQueue<T>::waitAndDeque(T &item)
{
    LOG_TRA("{}", queue_name_);

    std::unique_lock<std::mutex> lock(mutex_);

    if (!cond_not_empty_.wait_for(lock, std::chrono::seconds(kTimeoueSeconds), [this]
                                  { return count_ > 0; }))
    {
        return false;
    }

    item = buffer_[head_];
    head_ = nextIndex(head_);
    --count_;
    cond_not_full_.notify_one();
    return true;
}

template <typename T>
void ThreadSafeCircularQueue<T>::flush()
{
    LOG_DBG("Flushing {} (circular) queue", queue_name_);

    std::unique_lock<std::mutex> lock(mutex_);
    head_ = 0;
    tail_ = 0;
    count_ = 0;
    overwrite_count_ = 0;

    // 모두 비었으니 not_full 브로드캐스트 가능
    cond_not_full_.notify_all();
}

template <typename T>
size_t ThreadSafeCircularQueue<T>::nextIndex(size_t idx) const
{
    return (idx + 1) % capacity_;
}

} // namespace vp
