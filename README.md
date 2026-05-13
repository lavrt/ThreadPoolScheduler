# ThreadPoolScheduler

A multithreaded task processing system written in C++17. The project simulates asynchronous task execution using a thread pool, delayed task scheduling, and an asynchronous event-driven Logger.

## Introduction

The goal of this project is to develop a multithreaded system in C++17 that accepts a stream of incoming "tasks", processes them in parallel using a thread pool, and can gracefully shut down upon receiving a stop signal.

Key requirements:

- **Task Generation with Delays**:
Input data is not delivered instantly; it is received with a small delay.
Each task has a name (`taskName`), execution time (`payload`), and a startup delay (`delay`).
Payload and delay values are randomly assigned in the range 1–10 seconds.

- **Parallel Processing**:
The system creates N worker threads to execute M tasks concurrently.
Tasks are distributed among threads by the thread pool.

- **Logging and Statistics**:
Each task logs its start and completion time.
The system collects statistics for each worker, including the number of tasks executed and total payload.

- **Graceful Shutdown**:
Upon a stop signal, the system should stop accepting new delayed tasks.
Tasks already in the thread pool continue execution until completion.

## Project Architecture

The system consists of several independent components:

| Component | Description |
|---|---|
| `ThreadPool` | Creates and manages worker threads. Executes tasks that are ready for processing and collects per-worker statistics. |
| `Scheduler` | Stores delayed tasks and forwards them to the thread pool when their delay expires. |
| `ThreadSafeQueue` | A generic blocking queue used for communication between threads. It provides safe task/event passing without data races. |
| `Logger` | Runs in a dedicated thread and prints task execution events asynchronously. |
| `TaskGenerator` | Generates random tasks. Each task has a name, execution time (`payload`), and startup delay (`delay`). |

## Usage Examples

This section shows how to create the main components manually and how shutdown works.

### Running tasks directly in `ThreadPool`

`ThreadPool` starts worker threads in its constructor. Tasks submitted with `Enqueue()` are executed by one of the workers.

```cpp
#include <chrono>
#include <iostream>
#include <thread>

#include "thread_pool.h"

int main() {
    tps::thread_pool::ThreadPool pool{2};

    pool.Enqueue([] { // first task
        std::this_thread::sleep_for(std::chrono::seconds{1});
    });

    pool.Enqueue([] { // second task
        std::this_thread::sleep_for(std::chrono::seconds{2});
    });

    // waits until all already accepted tasks are finished
    pool.Wait();

    // stops accepting new tasks, wakes worker threads and joins them
    pool.Shutdown();
}
```

`Enqueue()` returns `false` after the pool has been stopped:

```cpp
pool.Shutdown();

bool accepted = pool.Enqueue([] {
    // this task will not run
});

if (!accepted) {
    std::cout << "pool is stopped\n";
}
```

### Scheduling delayed tasks with `Scheduler`

`Scheduler` owns a monitor thread. It stores delayed tasks and forwards them to `ThreadPool` when their activation time is reached.

```cpp
#include <chrono>
#include <iostream>

#include "scheduler.h"
#include "thread_pool.h"

int main() {
    using namespace std::chrono_literals;

    tps::thread_pool::ThreadPool pool{2};
    tps::scheduler::Scheduler scheduler{pool};

    auto now = tps::scheduler::Scheduler::Clock::now();

    scheduler.AddTask(now + 1s, [] {
        // runs after 1 second
    });

    scheduler.AddTask(now + 3s, [] {
        // runs after 3 seconds
    });

    // waits until the scheduler has forwarded all delayed tasks to the pool
    scheduler.Wait();

    // waits until the pool has finished executing forwarded tasks
    pool.Wait();

    scheduler.Stop();
    pool.Shutdown();
}
```

`Scheduler::Wait()` and `ThreadPool::Wait()` wait for different things:

- `scheduler.Wait()` waits until there are no delayed tasks left inside the scheduler.
- `pool.Wait()` waits until all tasks accepted by the thread pool are fully executed.

For that reason the usual order is:

```cpp
scheduler.Wait(); // all delayed tasks were submitted to the pool
pool.Wait();      // all submitted tasks finished execution
scheduler.Stop();
pool.Shutdown();
```

### What happens when `Scheduler` is stopped

`Scheduler::Stop()` stops the monitor thread and prevents new delayed tasks from being accepted. Tasks that are still waiting inside the scheduler are not forwarded to the pool after stop.

```cpp
#include <chrono>
#include <iostream>

#include "scheduler.h"
#include "thread_pool.h"

int main() {
    using namespace std::chrono_literals;

    tps::thread_pool::ThreadPool pool{1};
    tps::scheduler::Scheduler scheduler{pool};

    auto now = tps::scheduler::Scheduler::Clock::now();

    scheduler.AddTask(now + 10s, [] {
        // this task will probably not run
    });

    scheduler.Stop();

    bool accepted = scheduler.AddTask(now + 1s, [] {
        // this task will not be accepted
    });

    if (!accepted) {
        std::cout << "scheduler is stopped\n";
    }

    pool.Wait();
    pool.Shutdown();
}
```

### Graceful shutdown sequence

A graceful shutdown means:

1. Stop accepting new delayed tasks.
2. Let tasks that already reached the thread pool finish.
3. Join worker threads.
4. Print statistics or other final messages.

```cpp
scheduler.Stop(); // no more delayed tasks will be forwarded
pool.Wait();      // tasks already accepted by the pool continue until done
pool.Shutdown();  // joins worker threads
```

If the program should process every scheduled task before stopping, wait for the scheduler first:

```cpp
scheduler.Wait(); // wait until all delayed tasks are forwarded
pool.Wait();      // wait until all forwarded tasks finish
pool.Shutdown();
```

### Using `AsyncLogger` with the pool

`AsyncLogger` runs a separate logging thread. Calls to `Post()` only enqueue events; actual output is performed asynchronously.

```cpp
#include <chrono>
#include <iostream>
#include <thread>

#include "logger.h"
#include "thread_pool.h"

int main() {
    tps::logging::AsyncLogger logger{std::cout};
    tps::thread_pool::ThreadPool pool{2};

    pool.Enqueue([&logger] {
        int worker_id = tps::thread_pool::ThreadPool::worker_id;

        logger.Post(tps::logging::TaskStarted{
            worker_id,
            "example_task",
            tps::logging::Clock::now()
        });

        std::this_thread::sleep_for(std::chrono::seconds{1});

        logger.Post(tps::logging::TaskFinished{
            worker_id,
            "example_task",
            tps::logging::Clock::now()
        });
    });

    pool.Wait();
    pool.Shutdown();
}
```

`AsyncLogger` flushes all queued events in its destructor before the logging thread exits.

## Build and Run

### Requirements
- C++17 compatible compiler
- CMake 3.11+
- GoogleTest (optional)

### Clone the repository

```bash
git clone https://github.com/lavrt/ThreadPoolScheduler.git
cd ThreadPoolScheduler
```

### Build the project

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Run the application

```bash
./build/pool <threads_count> <tasks_count>
```

Example:

```bash
./build/pool 2 3
```

### Command Line Arguments

| Argument | Description |
|---|---|
| `threads_count` | Number of worker threads in the thread pool. Must be greater than 0. |
| `tasks_count` | Number of tasks to generate and process. Must be greater than 0. |

## Testing

Run unit tests with CTest:

```bash
ctest -V --test-dir build
```

## Example Output

```bash
3 tasks generated:
task_1 8 3
task_2 3 6
task_3 4 7
2 worker threads started
[Worker 1] task_1, started_at: 11:49:45
[Worker 2] task_2, started_at: 11:49:48
[Worker 2] task_2, finished_at: 11:49:51
[Worker 2] task_3, started_at: 11:49:51
[Worker 1] task_1, finished_at: 11:49:53
[Worker 2] task_3, finished_at: 11:49:55
All tasks are finished
Joining workers
Worker 1 is joined
Worker 2 is joined
Statistics:
Worker 1: made 1 tasks, total payload: 8 sec
Worker 2: made 2 tasks, total payload: 7 sec
```
