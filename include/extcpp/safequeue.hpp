#ifndef SAFE_QUEUE_HPP
#define SAFE_QUEUE_HPP

#include <queue>
#include <condition_variable>
#include <mutex>

namespace extcpp {

// Thread safe (re-entrant) Producer/Consumer queue.
// Supports multiple producers and multiple consumers.
// ---------------------------------------------------
template <typename T, typename Container = std::queue<T> >
class SafeQueue
{
public:
    using size_type = typename Container::size_type;

public:
    SafeQueue() = default;
    ~SafeQueue() = default;

    // Push an item on to the queue. Will notify any consumers waiting
    // for an item.
    void push(T const& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_.notify_one();
    }

    // Push an item on to the queue. Will notify any consumers waiting
    // for an item.
    void push(T&& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_.notify_one();
    }

    // Pop an item from the queue.
    // 	WARNING: This call will block if there are no items in the queue,
    // 	until an item is added.
    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(queue_.empty())
        {
            // release lock as long as the wait and reaquire it afterwards.
            cond_.wait(lock);
        }
        T val = queue_.front();
        queue_.pop();
        return val;
    }

    // The number of items currently in the queue.
    size_type size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    SafeQueue(const SafeQueue& other) = delete;
    SafeQueue& operator=(const SafeQueue& other) = delete;

private:
    Container queue_;
    std::condition_variable cond_;
    mutable std::mutex mutex_;
};

} // namespace extcpp

#endif
