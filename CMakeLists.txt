cmake_minimum_required(VERSION 3.11)
project(thread-pool-scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cl_parser ---------------------------------------

add_library(cl_parser STATIC
    src/cl_parser/cl_parser.cc
)

target_include_directories(cl_parser
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cl_parser
)

# logging -----------------------------------------

add_library(logging STATIC
    src/logging/event_output.cc
)

target_include_directories(logging
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logging
)

# core --------------------------------------------

add_library(core INTERFACE)

target_include_directories(core
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/task_generator
        ${CMAKE_CURRENT_SOURCE_DIR}/src/task
        ${CMAKE_CURRENT_SOURCE_DIR}/src/queue
        ${CMAKE_CURRENT_SOURCE_DIR}/src/thread_pool
        ${CMAKE_CURRENT_SOURCE_DIR}/src/scheduler
)

# pool --------------------------------------------

add_executable(pool
    src/main.cc
)

target_link_libraries(pool
    PRIVATE
        cl_parser
        logging
        core
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(pool PRIVATE -Wno-interference-size)
endif()
