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
        queue_.push(std::move(value));
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
        T val = std::move(queue_.front());
        queue_.pop();
        return val;
    }

    // WARNING: This is multi threaded code. Be wary of using size()
    //  since after the size() call has completed,
    //  in the case of many producers/consumers on different threads,
    //  already at the next instruction in the calling thread
    //  the size may have changed.
    //
    // USE WITH CARE!
    //
    // Example:
    //  auto size = queue.size();
    //  if(size != 0) <-- size may already have changed.
    //  {
    //   ...
    //  }
    size_type size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    // See the warning text about .size().
    // USE WITH CARE!
    bool empty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size() == 0;
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
