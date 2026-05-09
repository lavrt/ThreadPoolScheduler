#include <iostream>
#include <algorithm>

#include "cl_parser.h"
#include "task_generator.h"
#include "thread_pool.h"
#include "logger.h"

using namespace tps;

int main(int argc, const char* argv[]) {
    try {
        auto cfg = cl_parser::ParseCl(argc, argv);

        logging::AsyncLogger logger(std::cout);

        auto tasks = task_generator::TaskGenerator{}.Generate(cfg.task_count);
        
        logger.Post(logging::TasksGenerated{cfg.task_count});
        for (auto&& task : tasks) {
            logger.Post(logging::TaskInfo{task.name, task.payload, task.delay});
        }

        thread_pool::ThreadPool pool(cfg.thread_count, logger);
        for (auto&& task : tasks) {
            pool.Submit(task);
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
