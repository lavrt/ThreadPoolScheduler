#include <iostream>

#include "cl_parser.h"
#include "task_generator.h"

int main(int argc, const char* argv[]) {
    try {
        auto config = tps::cl_parser::ParseCl(argc, argv);
        
        auto tasks = tps::task_generator::TaskGenerator{}.Generate(config.task_count);

        for (auto&& task : tasks) {
            std::cout << task.name << " " << task.payload << " " << task.delay << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
