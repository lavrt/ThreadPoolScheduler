#pragma once

#include <ostream>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "events.h"

inline std::string FormatTime(std::chrono::system_clock::time_point time) {
    auto t = std::chrono::system_clock::to_time_t(time);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");

    return oss.str();
}

inline std::ostream& operator<<(std::ostream& os,
                                const tps::logging::TasksGenerated& event) {
    os << event.task_count << " tasks generated\n";
    return os;
}

inline std::ostream& operator<<(std::ostream& os,
                                const tps::logging::TaskInfo& event) {
    os << event.task_name << " "
       << event.task_payload << " "
       << event.task_delay << "\n";
    return os;
}

inline std::ostream& operator<<(std::ostream& os,
                                const tps::logging::ThreadsStarted& event) {
    os << event.thread_count << " worker threads started\n";
    return os;
}

inline std::ostream& operator<<(std::ostream& os,
                                const tps::logging::TaskStarted& event) {
    os << "[Worker " << event.worker_id << "] "
       << event.task_name << ", "
       << "started at: " << FormatTime(event.timestamp) << "\n";
    return os;
}

inline std::ostream& operator<<(std::ostream& os,
                                const tps::logging::TaskFinished& event) {
    os << "[Worker " << event.worker_id << "] "
       << event.task_name << ", "
       << "finished at: " << FormatTime(event.timestamp) << "\n";
    return os;
}
