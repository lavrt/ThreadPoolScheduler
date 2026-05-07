#pragma once

#include <string>

namespace tps::task {

struct Task {
    std::string name;
    int payload{};
    int delay{};
};

} // namespace tps::task
