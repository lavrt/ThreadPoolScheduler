#pragma once

struct ProgramConfig {
    long thread_count{};
    long task_count{};
};

ProgramConfig ParseCl(int argc, const char* argv[]);
