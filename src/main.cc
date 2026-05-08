#include <iostream>

#include "cl_parser.h"
#include "task_generator.h"
#include "thread_pool.h"
#include "logger.h"

int main(int argc, const char* argv[]) {
    try {
        auto config = tps::cl_parser::ParseCl(argc, argv);

        tps::logging::AsyncLogger logger;

        auto tasks = tps::task_generator::TaskGenerator{}.Generate(config.task_count);
        logger.Post(tps::logging::TasksGenerated{config.task_count});
        for (auto&& task : tasks) {
            logger.Post(tps::logging::TaskInfo{task.name, task.payload, task.delay});
        }

        tps::thread_pool::ThreadPool pool(config.thread_count);
        for (auto&& task : tasks) {
            pool.Submit(task);
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
