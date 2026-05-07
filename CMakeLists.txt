cmake_minimum_required(VERSION 3.11)
project(thread-pool-scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pool
    src/main.cc
    src/cl_parser/cl_parser.cc
)

target_include_directories(pool
    PRIVATE
        src/cl_parser
        src/task_generator
        src/task
)
