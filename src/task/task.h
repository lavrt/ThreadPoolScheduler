#pragma once

#include <string>
#include <chrono>

namespace tps::task {

struct Task {
    std::string name;
    int payload{};
    int delay{};
    std::chrono::steady_clock::time_point ready_at;
};

} // namespace tps::task
