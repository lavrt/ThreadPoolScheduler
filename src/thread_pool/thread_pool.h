#pragma once

#include <thread>
#include <vector>
#include <utility>
#include <thread>

#include "task_queue.h"

namespace tps::thread_pool {

class ThreadPool {
public:
    ThreadPool(int thread_count) : thread_count_(thread_count) {
        workers_.reserve(thread_count_);

        for (std::size_t i = 0, ie = thread_count_; i != ie; ++i) {
            workers_.emplace_back(&ThreadPool::WorkerLoop, this);
        }
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

    void WorkerLoop() {
        for (;;) {
            auto task_opt = tasks_.WaitAndPop();
            if (!task_opt.has_value()) {
                return;
            }

            auto task = std::move(task_opt.value());

            std::this_thread::sleep_for(std::chrono::seconds(task.payload));


        }
    }

private:
    int thread_count_;
    std::vector<std::thread> workers_;
    task_queue::TaskQueue tasks_;
};

} // namespace tps::thread_pool
