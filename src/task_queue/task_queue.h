#pragma once

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>

#include "task.h"

namespace tps::task_queue {

class TaskQueue {
public:
    TaskQueue(const TaskQueue&) = delete;

    TaskQueue& operator=(const TaskQueue&) = delete;

    void Push(task::Task task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            
            if (stopped_) {
                return;
            }

            tasks_.push(std::move(task));
        }

        condition_.notify_one();
    }

    std::optional<task::Task> WaitAndPop() {
        std::unique_lock<std::mutex> lock(mutex_);

        for (;;) {
            if (tasks_.empty()) {
                if (stopped_) {
                    return std::nullopt;
                }

                condition_.wait(lock, [this]{
                    return stopped_ || !tasks_.empty();
                });

                continue;
            }

            const auto ready_at = tasks_.top().ready_at;

            if (ready_at <= std::chrono::steady_clock::now()) {
                auto task = tasks_.top();
                tasks_.pop();
                return task;
            }

            condition_.wait_until(lock, ready_at);
        }
    }

    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }

        condition_.notify_all();
    }

private:
    struct CompareByReadyAt {
        bool operator()(const task::Task& lhs, const task::Task& rhs) const {
            return lhs.ready_at > rhs.ready_at;
        }
    };

    std::priority_queue<
        task::Task,
        std::vector<task::Task>,
        CompareByReadyAt
    > tasks_;

    std::mutex mutex_;
    std::condition_variable condition_;
    bool stopped_{false};
};

} // namespace tps::task_queue
