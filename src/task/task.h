#pragma once

#include <string>
#include <chrono>
#include <functional>

namespace tps::task {

struct Task {
    std::string name;
    int payload;
    int delay;
    std::function<void()> work;
    std::chrono::steady_clock::time_point ready_at;

    Task(std::string task_name,
         int task_payload,
         int task_delay,
         std::function<void()> task_work)
        : name(std::move(task_name)),
          payload(task_payload),
          delay(task_delay),
          work(std::move(task_work)),
          ready_at(std::chrono::steady_clock::now() +
                   std::chrono::seconds(task_delay)) {}

    void operator()() {
        work();
    }
};

} // namespace tps::task
