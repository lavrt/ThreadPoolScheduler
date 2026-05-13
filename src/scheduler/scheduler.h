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

    Scheduler(thread_pool::ThreadPool& pool) : pool_(pool) {
        monitor_thread_ = std::thread{&Scheduler::MonitorThreadLoop, this};
    }

    ~Scheduler() {
        Stop();
    }

    Scheduler(const Scheduler&) = delete;

    Scheduler& operator=(const Scheduler&) = delete;

    Scheduler(Scheduler&&) = delete;

    Scheduler& operator=(Scheduler&&) = delete;

    template <typename Function>
    bool AddTask(Clock::time_point ready_at, Function&& f) {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stopped_) {
                return false;
            }
            
            task_queue_.push(DelayedTask{ready_at, std::forward<Function>(f)});
        }

        cv_.notify_all();
        return true;
    }

    void Wait() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] {
                return stopped_ || task_queue_.empty();
            });
        }
    }

    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stopped_) {
                return;
            }
            
            stopped_ = true;
        }
        
        cv_.notify_all();

        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
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

    std::priority_queue<
        DelayedTask,
        std::vector<DelayedTask>,
        std::greater<DelayedTask>
    > task_queue_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_{false};

    std::thread monitor_thread_;

    void MonitorThreadLoop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        for (;;) {
            cv_.wait(lock, [this] {
                return !task_queue_.empty() || stopped_;
            });
            
            if (stopped_) {
                return;
            }

            const auto ready_at = task_queue_.top().activation_time;

            if (Clock::now() < ready_at) {
                cv_.wait_until(lock, ready_at, [this, ready_at] {
                    return stopped_ ||
                           task_queue_.empty() ||
                           task_queue_.top().activation_time < ready_at;
                });

                continue;
            }

            auto active_task = task_queue_.top();
            task_queue_.pop();
            
            pool_.Enqueue(std::move(active_task.task));
            
            cv_.notify_all();
        }
    }
};

} // namespace tps::scheduler
