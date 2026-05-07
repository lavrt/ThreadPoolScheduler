#pragma once

struct ProgramConfig {
    int thread_count{};
    int task_count{};
};

ProgramConfig ParseCl(int argc, const char* argv[]);
