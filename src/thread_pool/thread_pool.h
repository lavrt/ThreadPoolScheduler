#pragma once

#include <thread>
#include <vector>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <new>
#include <type_traits>

#include "thread_safe_queue.h"

namespace tps::thread_pool {

#ifdef __cpp_lib_hardware_interference_size
    using std::hardware_destructive_interference_size;
#else
    constexpr inline std::size_t hardware_destructive_interference_size = 64;
#endif

struct alignas(hardware_destructive_interference_size) WorkerStats {
    int id{};
    int tasks_done{};
    int total_payload{};
};

class ThreadPool {
public:
    using JoinCallback = std::function<void(int)>;

    static inline thread_local int worker_id{};

    ThreadPool(int thread_count) : stats_(thread_count) {
        for (int i = 0, ie = thread_count; i != ie; ++i) {
            threads_.emplace_back([this, id = i + 1] {
                worker_id = id;
                stats_[id - 1].id = id;
                while (auto task_opt = tasks_.WaitPop()) {
                    auto task_start = std::chrono::steady_clock::now();

                    try {
                        std::invoke(std::move(task_opt.value()));

                        auto task_end = std::chrono::steady_clock::now();
                        UpdateStats(id, task_end - task_start);
                    } catch (...) {}

                    NotifyTaskFinished();
                }
            });
        }
    }

    ~ThreadPool() {
        tasks_.Stop();

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

    template <
        typename Function,
        std::enable_if_t<
            std::is_invocable_r_v<void, std::decay_t<Function>&> &&
            std::is_constructible_v<std::function<void()>, Function&&>,
            int
        > = 0
    >
    bool Enqueue(Function&& f) {
        {
            std::lock_guard<std::mutex> lock(wait_mutex_);

            if (stopped_) {
                return false;
            }
            
            ++active_tasks_;
        }

        if (tasks_.Push(std::forward<Function>(f))) {
            return true;
        }

        NotifyTaskFinished();
        return false;
    }

    void Wait() {
        std::unique_lock<std::mutex> lock(wait_mutex_);
        wait_cv_.wait(lock, [this] {
            return active_tasks_ == 0;
        });
    }

    void Shutdown(JoinCallback on_join = nullptr) {
        {
            std::lock_guard<std::mutex> lock(wait_mutex_);
            stopped_ = true;
        }

        tasks_.Stop();

        for (std::size_t i = 0, ie = threads_.size(); i != ie; ++i) {
            if (threads_[i].joinable()) {
                threads_[i].join();
                if (on_join) {
                    int thread_id = static_cast<int>(i + 1);
                    on_join(thread_id);
                }
            }
        }
    }

    const std::vector<WorkerStats>& GetStats() const noexcept {
        return stats_;
    }

private:
    std::vector<std::thread> threads_;
    queue::ThreadSafeQueue<std::function<void()>> tasks_;

    std::vector<WorkerStats> stats_;

    std::mutex wait_mutex_;
    std::condition_variable wait_cv_;
    int active_tasks_{};

    bool stopped_{false};

    void NotifyTaskFinished() {
        std::lock_guard<std::mutex> lock(wait_mutex_);
        if (--active_tasks_ == 0) {
            wait_cv_.notify_all();
        }
    }

    void UpdateStats(int id, std::chrono::steady_clock::duration duration) {
        stats_[id - 1].total_payload +=
            std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        stats_[id - 1].tasks_done++;
    }
};

} // namespace tps::thread_pool
