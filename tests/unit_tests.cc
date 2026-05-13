#include <gtest/gtest.h>

#include <atomic>
#include <chrono>

#include "thread_pool.h"
#include "scheduler.h"

using namespace std::chrono_literals;

TEST(ThreadPoolTest, RunsOneTask) {
    tps::thread_pool::ThreadPool pool(1);

    std::atomic<int> x{0};

    ASSERT_TRUE(pool.Enqueue([&] {
        ++x;
    }));

    pool.Wait();

    EXPECT_EQ(x.load(), 1);

    pool.Shutdown();
}

TEST(ThreadPoolTest, RunsSeveralTasks) {
    tps::thread_pool::ThreadPool pool(2);

    std::atomic<int> x{0};

    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(pool.Enqueue([&] {
            ++x;
        }));
    }

    pool.Wait();

    EXPECT_EQ(x.load(), 10);

    pool.Shutdown();
}

TEST(ThreadPoolTest, DoesNotAddAfterShutdown) {
    tps::thread_pool::ThreadPool pool(1);

    pool.Shutdown();

    EXPECT_FALSE(pool.Enqueue([] {}));
}

TEST(ThreadPoolTest, WaitsForTask) {
    tps::thread_pool::ThreadPool pool(1);

    std::atomic<int> x{0};

    ASSERT_TRUE(pool.Enqueue([&] {
        std::this_thread::sleep_for(20ms);
        ++x;
    }));

    pool.Wait();

    EXPECT_EQ(x.load(), 1);

    pool.Shutdown();
}

TEST(SchedulerTest, RunsOneTask) {
    tps::thread_pool::ThreadPool pool(1);
    tps::scheduler::Scheduler scheduler(pool);

    std::atomic<int> x{0};

    ASSERT_TRUE(scheduler.AddTask(
        tps::scheduler::Scheduler::Clock::now(),
        [&] {
            ++x;
        }
    ));

    scheduler.Wait();
    pool.Wait();

    EXPECT_EQ(x.load(), 1);

    scheduler.Stop();
    pool.Shutdown();
}

TEST(SchedulerTest, RunsSeveralTasks) {
    tps::thread_pool::ThreadPool pool(2);
    tps::scheduler::Scheduler scheduler(pool);

    std::atomic<int> x{0};

    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(scheduler.AddTask(
            tps::scheduler::Scheduler::Clock::now(),
            [&] {
                ++x;
            }
        ));
    }

    scheduler.Wait();
    pool.Wait();

    EXPECT_EQ(x.load(), 5);

    scheduler.Stop();
    pool.Shutdown();
}

TEST(SchedulerTest, RunsDelayedTask) {
    tps::thread_pool::ThreadPool pool(1);
    tps::scheduler::Scheduler scheduler(pool);

    std::atomic<int> x{0};

    ASSERT_TRUE(scheduler.AddTask(
        tps::scheduler::Scheduler::Clock::now() + 20ms,
        [&] {
            ++x;
        }
    ));

    scheduler.Wait();
    pool.Wait();

    EXPECT_EQ(x.load(), 1);

    scheduler.Stop();
    pool.Shutdown();
}

TEST(SchedulerTest, DoesNotAddAfterStop) {
    tps::thread_pool::ThreadPool pool(1);
    tps::scheduler::Scheduler scheduler(pool);

    scheduler.Stop();

    EXPECT_FALSE(scheduler.AddTask(
        tps::scheduler::Scheduler::Clock::now(),
        [] {}
    ));

    pool.Shutdown();
}
