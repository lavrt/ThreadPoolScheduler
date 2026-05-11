#include "event_output.h"

#include <chrono>
#include <sstream>
#include <iomanip>
#include <string>
#include <ctime>

namespace tps::logging {

namespace {

std::string FormatTime(Clock::time_point time) {
    auto t = Clock::to_time_t(time);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");

    return oss.str();
}

} // namespace

std::ostream& operator<<(std::ostream& os, const TasksGenerated& event) {
    os << event.task_count << " tasks generated:\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const TaskInfo& event) {
    os << event.task_name << " "
       << event.task_payload << " "
       << event.task_delay << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ThreadsStarted& event) {
    os << event.thread_count << " worker threads started\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const TaskStarted& event) {
    os << "[Worker " << event.worker_id << "] "
       << event.task_name << ", "
       << "started_at: " << FormatTime(event.timestamp) << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const TaskFinished& event) {
    os << "[Worker " << event.worker_id << "] "
       << event.task_name << ", "
       << "finished_at: " << FormatTime(event.timestamp) << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const AllTasksFinished&) {
    os << "All tasks are finished\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const JoiningWorkers&) {
    os << "Joining workers\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const WorkerJoined& event) {
    os << "Worker " << event.worker_id << " is joined\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const StatisticsHeader&) {
    os << "Statistics:\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const WorkerStatistics& event) {
    os << "Worker " << event.worker_id << ": "
       << "made " << event.tasks_done << " tasks, "
       << "total payload: " << event.total_payload << " sec\n";
    return os;
}

} // namespace tps::logging
