#pragma once

#include <chrono>
#include <queue>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "thread_pool.h"

namespace tps::scheduler {

class Scheduler {
public:
    using Clock = std::chrono::steady_clock;

    Scheduler(thread_pool::ThreadPool& pool)
        : pool_(pool), monitor_thread_(&Scheduler::MonitorThreadLoop, this) {}

    ~Scheduler() {
        Stop();
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    Scheduler(const Scheduler&) = delete;

    Scheduler& operator=(const Scheduler&) = delete;

    Scheduler(Scheduler&&) = delete;

    Scheduler& operator=(Scheduler&&) = delete;

    template <typename Function>
    void AddTask(Clock::time_point ready_at, Function&& f) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            task_queue_.push(DelayedTask{ready_at, std::forward<Function>(f)});
        }
        cv_.notify_all();
    }

    void Wait() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] {
                return task_queue_.empty();
            });
        }
    }

    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

private:
    struct DelayedTask {
        Clock::time_point activation_time;
        std::function<void()> task;

        bool operator>(const DelayedTask& other) const {
            return activation_time > other.activation_time;
        }
    };

    thread_pool::ThreadPool& pool_;

    std::thread monitor_thread_;

    std::priority_queue<
        DelayedTask,
        std::vector<DelayedTask>,
        std::greater<DelayedTask>
    > task_queue_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_{false};

    void MonitorThreadLoop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        for (;;) {
            cv_.wait(lock, [this] {
                return !task_queue_.empty() || stopped_;
            });

            if (task_queue_.empty() && stopped_) {
                return;
            }

            const auto ready_at = task_queue_.top().activation_time;

            if (ready_at <= Clock::now()) {
                auto active_task = std::move(task_queue_.top());
                task_queue_.pop();
                
                lock.unlock();
                pool_.Enqueue(std::move(active_task.task));
                lock.lock();

                cv_.notify_all();

                continue;
            }

            cv_.wait_until(lock, ready_at);
        }
    }
};

} // namespace tps::scheduler
