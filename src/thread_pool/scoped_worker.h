#pragma once

#include <thread>
#include <utility>

#include "logger.h"

namespace tps::thread_pool {

class ScopedWorker {
public:
    ScopedWorker(std::thread t, int id, logging::AsyncLogger& logger)
        : t_(std::move(t)), id_(id), logger_(logger) {}

    ~ScopedWorker() {
        if (t_.joinable()) {
            t_.join();
            logger_.Post(logging::WorkerJoined{id_});
        }
    }

    ScopedWorker(const ScopedWorker&) = delete;
    
    ScopedWorker& operator=(const ScopedWorker&) = delete;

    ScopedWorker(ScopedWorker&&) = default;

    ScopedWorker& operator=(ScopedWorker&&) = delete;

    bool Joinable() const noexcept {
        return t_.joinable();
    }

private:
    std::thread t_;
    int id_;
    logging::AsyncLogger& logger_;
};

} // namespace tps::thread_pool
