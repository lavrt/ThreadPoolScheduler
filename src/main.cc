#include <iostream>
#include <thread>
#include <chrono>

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
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            pool.Submit(task);
        }

        pool.Shutdown();

        logger.Post(logging::StatisticsHeader{});
        const auto& stats = pool.GetStats();
        for (std::size_t i = 0, ie = stats.size(); i != ie; ++i) {
            logger.Post(logging::WorkerStatistics{
                static_cast<int>(i + 1),
                stats[i].tasks_done,
                stats[i].total_payload
            });
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
