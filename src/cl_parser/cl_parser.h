#pragma once

namespace tps::cl_parser {

struct ProgramConfig {
    int thread_count{};
    int task_count{};
};

ProgramConfig ParseCl(int argc, const char* argv[]);

} // namespace tps::cl_parser
