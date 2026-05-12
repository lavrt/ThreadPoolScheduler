#include <iostream>

#include "cl_parser.h"
#include "task_generator.h"
#include "thread_pool.h"
#include "logger.h"
#include "scheduler.h"

using namespace tps;

int main(int argc, const char* argv[]) {
    try {
        auto cfg = cl_parser::ParseCl(argc, argv);

        logging::AsyncLogger logger{std::cout};

        auto tasks = task_generator::TaskGenerator{}.Generate(cfg.task_count);
        logger.Post(logging::TasksGenerated{cfg.task_count});
        for (auto&& task : tasks) {
            logger.Post(logging::TaskInfo{task.name, task.payload, task.delay});
        }

        thread_pool::ThreadPool pool{cfg.thread_count};
        logger.Post(logging::ThreadsStarted{cfg.thread_count});

        scheduler::Scheduler scheduler{pool};

        for (auto&& task : tasks) {
            scheduler.AddTask(task.ready_at,
                [task = std::move(task), &logger]() mutable {
                    auto id = thread_pool::ThreadPool::worker_id;

                    logger.Post(logging::TaskStarted{
                        id,
                        task.name,
                        logging::Clock::now()
                    });

                    task();

                    logger.Post(logging::TaskFinished{
                        id,
                        task.name,
                        logging::Clock::now()
                    });
                }
            );
        }

        scheduler.Wait();
        pool.Wait();

        logger.Post(logging::AllTasksFinished{});

        logger.Post(logging::JoiningWorkers{});
        pool.Shutdown([&logger](int id) {
            logger.Post(logging::WorkerJoined{id});
        });

        logger.Post(logging::StatisticsHeader{});
        auto stats = pool.GetStats();
        for (std::size_t i = 0, ie = cfg.thread_count; i != ie; ++i) {
            logger.Post(logging::WorkerStatistics{
                stats[i].id,
                stats[i].tasks_done,
                stats[i].total_payload
            });
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
