#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace autocrypt
{

template <typename Key, typename Value, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
class LruCache
{
public:
    LruCache(uint64_t max_size = 1000)
        : max_size_(max_size) {}

    uint64_t size() const { return cache_.size(); }

    uint64_t capacity() const { return max_size_; }

    bool isEmpty() const { return cache_.empty(); }

    bool contains(const Key &key) const { return cache_.find(key) != cache_.end(); }

    void put(const Key &key, const Value &value)
    {
        std::lock_guard<std::mutex> lock{mutex_};

        auto it = cache_.find(key);
        if (it != cache_.end())
        {
            it->second.value = value;
            usage_.erase(it->second.iterator);
            usage_.push_back(key);
            it->second.iterator = std::prev(usage_.end());
        }
        else
        {
            while (cache_.size() >= max_size_)
            {
                auto oldest_key = usage_.front();
                cache_.erase(usage_.front());
                usage_.pop_front();
            }
            usage_.push_back(key);
            cache_.emplace(key, CacheEntry{value, std::prev(usage_.end())});
        }
    }

    void put(const Key &key, Value &&value)
    {
        std::lock_guard<std::mutex> lock{mutex_};

        auto it = cache_.find(key);
        if (it != cache_.end())
        {
            it->second.value = std::move(value);
            usage_.erase(it->second.iterator);
            usage_.push_back(key);
            it->second.iterator = std::prev(usage_.end());
        }
        else
        {
            while (cache_.size() >= max_size_)
            {
                auto oldest_key = usage_.front();
                cache_.erase(usage_.front());
                usage_.pop_front();
            }
            usage_.push_back(key);
            CacheEntry entry{std::move(value), std::prev(usage_.end())};
            cache_.emplace(key, std::move(entry));
        }
    }

    std::optional<std::reference_wrapper<const Value>> get(const Key &key)
    {
        std::lock_guard<std::mutex> lock{mutex_};

        auto it = cache_.find(key);
        if (it != cache_.end())
        {
            usage_.erase(it->second.iterator);
            usage_.push_back(key);
            it->second.iterator = std::prev(usage_.end());
            return std::cref(it->second.value);
        }

        return std::nullopt;
    }

    void remove(const Key &key)
    {
        std::lock_guard<std::mutex> lock{mutex_};
        auto it = cache_.find(key);
        if (it != cache_.end())
        {
            usage_.erase(it->second.iterator);
            cache_.erase(key);
        }
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock{mutex_};
        cache_.clear();
        usage_.clear();
    }

private:
    struct CacheEntry
    {
        using iterator_type = typename std::list<Key>::iterator;
        Value value;            // Value
        iterator_type iterator; // Iterator to the usage list
    };
    uint64_t max_size_;
    std::unordered_map<Key,
                       CacheEntry,
                       Hash,
                       KeyEqual>
        cache_;
    std::list<Key> usage_;
    std::mutex mutex_;
};

} // namespace autocrypt