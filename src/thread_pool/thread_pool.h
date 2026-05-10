#pragma once

#include <thread>
#include <vector>
#include <utility>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <algorithm>
#include <new>

#include "task_queue.h"
#include "logger.h"

namespace tps::thread_pool {

struct alignas(std::hardware_destructive_interference_size) WorkerStats {
    int tasks_done{};
    int total_payload{};
};

class ThreadPool {
public:
    ThreadPool(int thread_count, logging::AsyncLogger& logger)
        : thread_count_(thread_count),
          stats_(static_cast<std::size_t>(thread_count)),
          logger_(logger) {
        try {
            workers_.reserve(thread_count_);
        
            for (int id = 1; id <= thread_count_; ++id) {
                workers_.emplace_back(&ThreadPool::WorkerLoop, this, id);
            }

            SafePost(logging::ThreadsStarted{thread_count_});
        } catch (...) {
            tasks_.Stop();
            for (auto&& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
            throw;
        }
    }

    ~ThreadPool() {
        Shutdown();
    }

    ThreadPool(const ThreadPool&) = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) = delete;

    ThreadPool& operator=(ThreadPool&&) = delete;

    void Submit(task::Task task) {
        tasks_.Push(std::move(task));
    }

    void Shutdown() {
        if (is_shutdown_.exchange(true)) {
            return;
        }

        tasks_.Stop();

        {
            std::unique_lock<std::mutex> lock(exit_mutex_);
            exit_cv_.wait(lock, [this] {
                return exited_workers_ == workers_.size();
            });
        }

        SafePost(logging::AllTasksFinished{});

        Join();
    }

    const std::vector<WorkerStats>& GetStats() const noexcept {
        return stats_;
    }

private:
    int thread_count_;
    std::vector<std::thread> workers_;
    task_queue::TaskQueue tasks_;

    std::vector<WorkerStats> stats_;

    std::atomic_bool is_shutdown_{false};

    int exited_workers_{};
    std::mutex exit_mutex_;
    std::condition_variable exit_cv_;

    logging::AsyncLogger& logger_;

    void Join() {
        if (std::none_of(workers_.begin(), workers_.end(), [](auto&& w) {
                return w.joinable();
            })
        ) return;

        SafePost(logging::JoiningWorkers{});

        for (int i = 0; i != workers_.size(); ++i) {
            if (workers_[i].joinable()) {
                workers_[i].join();
                SafePost(logging::WorkerJoined{i + 1});
            }
        }
    }

    void WorkerLoop(int worker_id) {
        while (auto task_opt = tasks_.WaitAndPop()) {
            auto task = std::move(task_opt.value());

            SafePost(logging::TaskStarted{
                worker_id,
                task.name,
                std::chrono::system_clock::now()});

            std::this_thread::sleep_for(std::chrono::seconds(task.payload));

            stats_[static_cast<std::size_t>(worker_id - 1)].tasks_done++;
            stats_[static_cast<std::size_t>(worker_id - 1)].total_payload
                += task.payload;

            SafePost(logging::TaskFinished{
                worker_id,
                task.name,
                std::chrono::system_clock::now()});
        }
        
        {
            std::lock_guard<std::mutex> lock(exit_mutex_);
            ++exited_workers_;
        }
        exit_cv_.notify_all();
    }

    template <typename Event>
    void SafePost(Event&& event) noexcept {
        try {
            logger_.Post(std::forward<Event>(event));
        } catch (...) {
            // suppress exceptions
        }
    }
};

} // namespace tps::thread_pool
