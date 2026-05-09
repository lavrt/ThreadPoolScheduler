#pragma once

#include <thread>
#include <vector>
#include <utility>
#include <thread>
#include <atomic>

#include "task_queue.h"
#include "logger.h"

namespace tps::thread_pool {

class ThreadPool {
public:
    ThreadPool(int thread_count, logging::AsyncLogger& logger)
        : thread_count_(thread_count), logger_(logger) {
        workers_.reserve(thread_count_);

        for (std::size_t i = 0, ie = thread_count_; i != ie; ++i) {
            workers_.emplace_back(&ThreadPool::WorkerLoop, this, i + 1);
        }

        logger.Post(logging::ThreadsStarted{thread_count_});
    }

    ~ThreadPool() {
        Stop();
        Join();
    }

    ThreadPool(const ThreadPool&) = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) = delete;

    ThreadPool& operator=(ThreadPool&&) = delete;

    void Submit(task::Task task) {
        tasks_.Push(std::move(task));
    }

    void Join() {
        for (auto&& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void Stop() {
        tasks_.Stop();
    }

    void WorkerLoop(int worker_id) {
        for (;;) {
            auto task_opt = tasks_.WaitAndPop();
            if (!task_opt.has_value()) {
                return;
            }

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
    }

private:
    int thread_count_;
    std::vector<std::thread> workers_;
    task_queue::TaskQueue tasks_;
    logging::AsyncLogger& logger_;
};

} // namespace tps::thread_pool
