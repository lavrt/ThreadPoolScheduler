#pragma once

#include <variant>
#include <string>
#include <chrono>

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

struct TaskStarted {
    int worker_id{};
    std::string task_name;
    std::chrono::system_clock::time_point timestamp;
};

struct TaskFinished {
    int worker_id{};
    std::string task_name;
    std::chrono::system_clock::time_point timestamp;
};

struct AllTasksFinished {};

struct JoiningWorkers {};

struct WorkerJoined {
    int worker_id{};
};

using Event = std::variant<
    TasksGenerated,
    TaskInfo,
    ThreadsStarted,
    TaskStarted,
    TaskFinished,
    AllTasksFinished,
    JoiningWorkers,
    WorkerJoined
>;

} // namespace tps::logging
