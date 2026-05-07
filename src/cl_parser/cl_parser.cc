#include <stdexcept>
#include <string>

#include "cl_parser.h"

ProgramConfig ParseCl(int argc, const char* argv[]) {
    if (argc != 3) {
        throw std::runtime_error("Usage: " + std::string{argv[0]} + " N M");
    }

    ProgramConfig config;

    try {
        config = {.thread_count = std::stol(argv[1]),
                  .task_count = std::stol(argv[2])};
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid arguments");
    }

    if (config.thread_count <= 0 || config.task_count <= 0) {
        throw std::runtime_error("N and M must be greater than 0");
    }

    return config;
}
