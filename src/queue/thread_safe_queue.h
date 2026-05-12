#pragma once

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <type_traits>

namespace tps::queue {

template <typename T,
          typename = std::enable_if_t<std::is_move_constructible_v<T>>>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    ~ThreadSafeQueue() = default;

    ThreadSafeQueue(const ThreadSafeQueue&) = delete;

    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    ThreadSafeQueue(ThreadSafeQueue&&) = delete;

    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

    void Push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) {
                return;
            }
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }

    std::optional<T> WaitPop() {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this] {
            return !queue_.empty() || stopped_;
        });

        if (queue_.empty() && stopped_) {
            return std::nullopt;
        }

        T value = std::move(queue_.front());
        queue_.pop();

        return value;
    }

    std::optional<T> TryPop() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (queue_.empty()) {
            return std::nullopt;
        }

        T value = std::move(queue_.front());
        queue_.pop();

        return value;
    }

    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    std::size_t Size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_{false};
};

} // namespace tps::queue
