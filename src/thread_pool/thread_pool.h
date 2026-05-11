#pragma once

#include <thread>
#include <vector>
#include <utility>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <new>

#include "thread_safe_queue.h"
#include "logger.h"

namespace tps::thread_pool {

struct alignas(std::hardware_destructive_interference_size) WorkerStats {
    int id{};
    int tasks_done{};
    int total_payload{};
};

class ThreadPool {
public:
    using JoinCallback = std::function<void(int)>;

    static inline thread_local int worker_id;

    ThreadPool(int thread_count) : stats_(thread_count) {
        for (int i = 0, ie = thread_count; i != ie; ++i) {
            threads_.emplace_back([this, id = i + 1] {
                worker_id = id;
                while (auto task_opt = tasks_.WaitPop()) {
                    auto task_start = std::chrono::steady_clock::now();

                    std::invoke(std::move(task_opt.value()));

                    auto task_end = std::chrono::steady_clock::now();

                    stats_[id - 1].total_payload += std::chrono::duration_cast<
                        std::chrono::seconds>(task_end - task_start).count();
                    stats_[id - 1].tasks_done++;

                    NotifyTaskFinished();
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

    void Shutdown(JoinCallback on_join = nullptr) {
        tasks_.Close();

        for (std::size_t i = 0, ie = threads_.size(); i != ie; ++i) {
            if (threads_[i].joinable()) {
                threads_[i].join();
                if (on_join) {
                    on_join(static_cast<int>(i + 1));
                }
            }
        }
    }

    const std::vector<WorkerStats>& GetStats() const {
        return stats_;
    }

private:
    std::vector<std::thread> threads_;
    queue::ThreadSafeQueue<std::function<void()>> tasks_;

    std::vector<WorkerStats> stats_;

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
