#include <stdexcept>
#include <string>

#include "cl_parser.h"

namespace tps::cl_parser {

ProgramConfig ParseCl(int argc, const char* argv[]) {
    if (argc != 3) {
        throw std::runtime_error("Usage: " + std::string{argv[0]} + " N M");
    }

    ProgramConfig config;

    try {
        config = {std::stoi(argv[1]), std::stoi(argv[2])};
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid arguments: " + std::string{e.what()});
    }

    if (config.thread_count <= 0 || config.task_count <= 0) {
        throw std::runtime_error("N and M must be greater than 0");
    }

    return config;
}

} // namespace tps::cl_parser
