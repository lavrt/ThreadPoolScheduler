#pragma once

#include <ostream>
#include <thread>
#include <queue>
#include <mutex>
#include <utility>
#include <condition_variable>
#include <variant>

#include "events.h"
#include "event_output.h"

namespace tps::logging {

class AsyncLogger {
public:
    AsyncLogger(std::ostream& os) : os_(os) {
        log_thread_ = std::thread{&AsyncLogger::ProcessLogs, this};
    }

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

    bool Post(Event event) {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stopped_) {
                return false;
            }

            events_.push(std::move(event));
        }
        
        cv_.notify_one();
        return true;
    }

private:
    std::ostream& os_;

    std::queue<Event> events_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_{false};

    std::thread log_thread_;

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
            std::visit([this](auto&& ev) {
                os_ << ev;
            }, event);
            lock.lock();
        }
    }
};

} // namespace tps::logging
