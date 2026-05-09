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

namespace tps::thread_pool {

class ThreadPool {
public:
    ThreadPool(int thread_count, logging::AsyncLogger& logger)
        : thread_count_(thread_count), logger_(logger) {
        workers_.reserve(thread_count_);

        for (int i = 0, ie = thread_count_; i != ie; ++i) {
            workers_.emplace_back(&ThreadPool::WorkerLoop, this, i + 1);
        }

        logger.Post(logging::ThreadsStarted{thread_count_});
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
                return exited_workers_ == thread_count_;
            });
        }

        logger_.Post(logging::AllTasksFinished{});

        logger_.Post(logging::JoiningWorkers{});

        for (int i = 0; i != workers_.size(); ++i) {
            if (workers_[i].joinable()) {
                workers_[i].join();
                logger_.Post(logging::WorkerJoined{i + 1});
            }
        }
    }

private:
    int thread_count_;
    std::vector<std::thread> workers_;
    task_queue::TaskQueue tasks_;

    std::atomic_bool is_shutdown_{false};

    int exited_workers_{};
    std::mutex exit_mutex_;
    std::condition_variable exit_cv_;

    logging::AsyncLogger& logger_;

    void WorkerLoop(int worker_id) {
        while (auto task_opt = tasks_.WaitAndPop()) {
            auto task = std::move(task_opt.value());

            logger_.Post(logging::TaskStarted{
                worker_id,
                task.name,
                std::chrono::system_clock::now()});

            std::this_thread::sleep_for(std::chrono::seconds(task.payload));

            logger_.Post(logging::TaskFinished{
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
};

} // namespace tps::thread_pool
