#pragma once

#include <ostream>

#include "events.h"

namespace tps::logging {

std::ostream& operator<<(std::ostream& os, const TasksGenerated& event);
std::ostream& operator<<(std::ostream& os, const TaskInfo& event);
std::ostream& operator<<(std::ostream& os, const ThreadsStarted& event);
std::ostream& operator<<(std::ostream& os, const TaskStarted& event);
std::ostream& operator<<(std::ostream& os, const TaskFinished& event);
std::ostream& operator<<(std::ostream& os, const AllTasksFinished&);
std::ostream& operator<<(std::ostream& os, const JoiningWorkers&);
std::ostream& operator<<(std::ostream& os, const WorkerJoined& event);
std::ostream& operator<<(std::ostream& os, const StatisticsHeader&);
std::ostream& operator<<(std::ostream& os, const WorkerStatistics& event);

} // namespace tps::logging
