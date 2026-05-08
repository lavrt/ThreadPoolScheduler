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

} // namespace tps::logging
