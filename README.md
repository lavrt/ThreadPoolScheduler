# ThreadPoolScheduler

A multithreaded task processing system written in C++17. The project simulates asynchronous task execution using a thread pool, delayed task scheduling, and an asynchronous event-driven Logger.

## Introduction

The goal of this project is to develop a multithreaded system in C++17 that accepts a stream of incoming "tasks", processes them in parallel using a thread pool, and can gracefully shut down upon receiving a stop signal.

Key requirements:

1) **Task Generation with Delays**:
Input data is not delivered instantly; it is received with a small delay.
Each task has a name (`taskName`), execution time (`payload`), and a startup delay (`delay`).
Payload and delay values are randomly assigned in the range 1–10 seconds.

2) **Parallel Processing**:
The system creates N worker threads to execute M tasks concurrently.
Tasks are distributed among threads by the thread pool.

3) **Logging and Statistics**:
Each task logs its start and completion time.
The system collects statistics for each worker, including the number of tasks executed and total payload.

4) **Graceful Shutdown**:
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
