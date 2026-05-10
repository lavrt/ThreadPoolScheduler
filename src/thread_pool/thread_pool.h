#pragma once

#include <thread>
#include <vector>
#include <utility>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "task_queue.h"
#include "logger.h"
#include "stats.h"

namespace tps::thread_pool {

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
            Shutdown();
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

    bool Submit(task::Task task) {
        {
            std::lock_guard<std::mutex> lock(wait_mutex);
            ++unfinished_tasks_;
        }

        if (!tasks_.Push(std::move(task))) {
            {
                std::lock_guard<std::mutex> lock(wait_mutex);
                --unfinished_tasks_;
            }
            wait_cv_.notify_all();
            return false;
        }

        return true;
    }

    // GetStats() is valid only after Shutdown()
    const stats::StatsCollector& GetStats() const noexcept {
        return stats_;
    }

    void Wait() {
        std::unique_lock<std::mutex> lock(wait_mutex);
        wait_cv_.wait(lock, [this] {
            return unfinished_tasks_ == 0;
        });
    }
    
    void Shutdown() {
        tasks_.Stop();
        JoinWorkers();
    }

private:
    int thread_count_;
    std::vector<std::thread> workers_;
    task_queue::TaskQueue<task::Task> tasks_;

    stats::StatsCollector stats_;

    std::size_t unfinished_tasks_{};
    std::condition_variable wait_cv_;
    std::mutex wait_mutex;

    logging::AsyncLogger& logger_;

    void JoinWorkers() {
        for (std::size_t i = 0, ie = workers_.size(); i != ie; ++i) {
            if (workers_[i].joinable()) {
                workers_[i].join();
                SafePost(logging::WorkerJoined{static_cast<int>(i + 1)});
            }
        }
    }

    void WorkerLoop(int worker_id) {
        while (auto task_opt = tasks_.WaitAndPop()) {
            auto task = std::move(task_opt.value());

            SafePost(logging::TaskStarted{
                worker_id,
                task.name,
                std::chrono::system_clock::now()
            });

            try {
                task();

                SafePost(logging::TaskFinished{
                    worker_id,
                    task.name,
                    std::chrono::system_clock::now()
                });

                stats_.TaskDone(worker_id - 1);
                stats_.AddPayload(worker_id - 1, task.payload);
            } catch (...) {
                // suppress exceptions
            }

            {
                std::lock_guard<std::mutex> lock(wait_mutex);
                --unfinished_tasks_;
            }
            wait_cv_.notify_all();
        }
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
