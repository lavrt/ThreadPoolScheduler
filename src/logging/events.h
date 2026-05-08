#pragma once

namespace tps::logging {

struct TasksGenerated {
    int task_count{};
};

struct TaskInfo {
    std::string task_name;
    int task_payload{};
    int task_delay{};
};

struct ThreadsStarted {
    int thread_count{};
};

using Event = std::variant<
    TasksGenerated,
    TaskInfo,
    ThreadsStarted
>;

} // namespace tps::logging
