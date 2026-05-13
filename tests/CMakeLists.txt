cmake_minimum_required(VERSION 3.11)

project(pool_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(GTest REQUIRED)

add_executable(unit_tests
    unit_tests.cc
)

target_link_libraries(unit_tests
    PRIVATE
        core
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME unit_tests COMMAND unit_tests)
