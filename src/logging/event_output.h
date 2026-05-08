#pragma once

#include <ostream>

#include "events.h"

inline std::ostream& operator<<(std::ostream& os, const tps::logging::TasksGenerated& event) {
    os << event.task_count << " tasks generated:\n";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const tps::logging::TaskInfo& event) {
    os << event.task_name << " " << event.task_payload << " " << event.task_delay << "\n";
    return os;
}
