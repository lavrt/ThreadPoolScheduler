#pragma once

#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <cstddef>
#include <thread>

#include "task.h"

namespace tps::task_generator {

class TaskGenerator {
public:
    TaskGenerator(int min_seconds = 1, int max_seconds = 10)
        : distribution_(min_seconds, max_seconds),
          random_engine_(std::random_device{}()) {}

    std::vector<task::Task> Generate(std::size_t task_count) {
        std::vector<task::Task> tasks;
        tasks.reserve(task_count);

        for (std::size_t i = 0, ie = task_count; i != ie; ++i) {
            auto payload = GetRandomInt();
            auto delay = GetRandomInt();

            task::Task task {
                "task_" + std::to_string(i + 1),
                payload,
                delay,
                [payload] {
                    std::this_thread::sleep_for(std::chrono::seconds(payload));
                }
            };

            tasks.push_back(std::move(task));
        }

        return tasks;
    }

private:
    std::uniform_int_distribution<int> distribution_;
    std::mt19937 random_engine_;

    int GetRandomInt() {
        return distribution_(random_engine_);
    }
};

} // namespace tps::task_generator
