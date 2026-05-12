# ThreadPoolScheduler

A multithreaded task processing system written in C++17.
The project simulates asynchronous task execution using a thread pool, delayed task scheduling, and an asynchronous event-driven Logger.

## Project Architecture

The system consists of several independent components:

### ThreadPool

Manages worker threads and distributes tasks between them.

### Scheduler

Tracks delayed tasks and sends them to the thread pool once their delay expires.

### ThreadSafeQueue

A generic blocking queue implementation used for communication between threads.

### Logger

Processes logging events asynchronously in a dedicated thread.

### TaskGenerator

Generates random tasks with:

- task name
- execution time (`payload`)
- startup delay (`delay`)

## Build

### Requirements
- C++17 compatible compiler
- CMake 3.11+

### Get the code

```bash
git clone https://github.com/lavrt/ThreadPoolScheduler.git
cd ThreadPoolScheduler
```

### Build Commands

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage

```bash
./build/pool <threads_count> <tasks_count>
```

Example:

```bash
./build/pool 2 4
```

## Example Output

```bash
4 tasks generated:
task_1 5 5
task_2 4 4
task_3 10 8
task_4 7 8
2 worker threads started
[Worker 1] task_2, started_at: 17:56:59
[Worker 2] task_1, started_at: 17:57:00
[Worker 1] task_2, finished_at: 17:57:03
[Worker 1] task_3, started_at: 17:57:03
[Worker 2] task_1, finished_at: 17:57:05
[Worker 2] task_4, started_at: 17:57:05
[Worker 2] task_4, finished_at: 17:57:12
[Worker 1] task_3, finished_at: 17:57:13
All tasks are finished
Joining workers
Worker 1 is joined
Worker 2 is joined
Statistics:
Worker 1: made 2 tasks, total payload: 14 sec
Worker 2: made 2 tasks, total payload: 12 sec
```
