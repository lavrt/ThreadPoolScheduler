#pragma once

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <utility>
#include <condition_variable>
#include <variant>

#include "events.h"
#include "event_output.h"

namespace tps::logging {

using Event = std::variant<
    TasksGenerated,
    TaskInfo
>;

class AsyncLogger {
public:
    AsyncLogger() : log_thread_(&AsyncLogger::ProcessLogs, this) {}

    ~AsyncLogger() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }

        cv_.notify_all();
        
        if (log_thread_.joinable()) {
            log_thread_.join();
        }
    }

    void Post(Event event) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            events_.push(std::move(event));
        }
        
        cv_.notify_one();
    }

private:
    std::thread log_thread_;
    std::queue<Event> events_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_{false};

    void ProcessLogs() {
        std::unique_lock<std::mutex> lock(mutex_);

        for (;;) {
            cv_.wait(lock, [this] {
                return stopped_ || !events_.empty();
            });

            if (stopped_ && events_.empty()) {
                return;
            }

            auto event = std::move(events_.front());
            events_.pop();

            lock.unlock();
            std::visit([](auto&& ev) {
                std::cout << ev;
            }, event);
            lock.lock();
        }
    }
};

} // namespace tps::logging
