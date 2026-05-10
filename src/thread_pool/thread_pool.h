#pragma once

#include <thread>
#include <vector>
#include <utility>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>

#include "task_queue.h"
#include "logger.h"
#include "stats.h"

namespace tps::thread_pool {

struct /* alignas(std::hardware_destructive_interference_size) */ WorkerContext {
    int id{};
    int tasks_done{};
    int total_payload{};
};

class ThreadPool {

public:
    static inline thread_local int worker_id;
    static inline thread_local WorkerContext worker_context;

    ThreadPool(int thread_count) : thread_count_(thread_count), stats_(thread_count) {
        for (int i = 0, ie = thread_count_; i != ie; ++i) {
            threads_.emplace_back([this, i] {
                worker_id = i + 1;
                while (auto task_opt = tasks_.WaitPop()) {
                    std::invoke(std::move(task_opt.value()));
                    NotifyTaskFinished();
                    stats_[i] = worker_context;
                }
            });
        }
    }

    ~ThreadPool() {
        tasks_.Close();

        for (auto&& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    ThreadPool(const ThreadPool&) = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) = delete;

    ThreadPool& operator=(ThreadPool&&) = delete;

    template <typename Function>
    void Enqueue(Function&& f) {
        {
            std::lock_guard<std::mutex> lock(wait_mutex_);
            ++active_tasks_;
        }
        tasks_.Push(std::forward<Function>(f));
    }

    void Wait() {
        std::unique_lock<std::mutex> lock(wait_mutex_);
        wait_cv_.wait(lock, [this] {
            return active_tasks_ == 0;
        });
    }

    const std::vector<WorkerContext>& GetStats() const {
        return stats_;
    }

private:
    int thread_count_;
    std::vector<std::thread> threads_;
    task_queue::ThreadSafeQueue<std::function<void()>> tasks_;

    std::vector<WorkerContext> stats_;

    std::mutex wait_mutex_;
    std::condition_variable wait_cv_;
    int active_tasks_{};

    void NotifyTaskFinished() {
        std::lock_guard<std::mutex> lock(wait_mutex_);
        if (--active_tasks_ == 0) {
            wait_cv_.notify_all();
        }
    }
};

} // namespace tps::thread_pool
