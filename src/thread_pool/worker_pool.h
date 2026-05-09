#pragma once 

#include <algorithm>
#include <functional>
#include <vector>

#include "logger.h"
#include "scoped_worker.h"

namespace tps::thread_pool {
    
class WorkerPool {
public:
    WorkerPool(logging::AsyncLogger& logger) : logger_(logger) {}

    ~WorkerPool() {
        JoinAll();
    }

    WorkerPool(const WorkerPool&) = delete;

    WorkerPool& operator=(const WorkerPool&) = delete;

    WorkerPool(WorkerPool&&) = delete;

    WorkerPool& operator=(WorkerPool&&) = delete;

    template <typename F, typename... Args>
    auto Add(int id, F&& func, Args&&... args)
        -> std::enable_if_t<std::is_invocable_v<F, Args...>> {
        workers_.emplace_back(
            std::thread{std::forward<F>(func), std::forward<Args>(args)...},
            id,
            logger_);
    }

    void JoinAll() {
        if (HasJoinableWorkers()) {
            logger_.Post(logging::JoiningWorkers{});
            workers_.clear();
        }
    }

    void Reserve(std::size_t n) {
        workers_.reserve(n);
    }

    bool HasJoinableWorkers() const {
        return std::any_of(workers_.begin(), workers_.end(), [](auto&& w) {
            return w.Joinable();
        });
    }

private:
    std::vector<ScopedWorker> workers_;
    logging::AsyncLogger& logger_;
};

} // namespace tps::thread_pool
