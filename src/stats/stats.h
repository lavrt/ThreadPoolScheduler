#pragma once

#include <vector>
#include <new>
#include <cstddef>

namespace tps::stats {

struct alignas(std::hardware_destructive_interference_size) WorkerStats {
    int tasks_done{};
    int total_payload{};
};

class StatsCollector {
public:
    StatsCollector(std::size_t worker_count) : stats_(worker_count) {}

    void TaskDone(std::size_t i) noexcept {
        ++stats_[i].tasks_done;
    }

    void AddPayload(std::size_t i, int add_payload) noexcept {
        stats_[i].total_payload += add_payload;
    }

    WorkerStats& operator[](std::size_t i) noexcept {
        return stats_[i];
    }

    const WorkerStats& operator[](std::size_t i) const noexcept {
        return stats_[i];
    }

    std::size_t Size() const noexcept {
        return stats_.size();
    }

private:
    std::vector<WorkerStats> stats_;
};

} // namespace tps::stats
