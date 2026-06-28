cmake_minimum_required(VERSION 3.16)

project(ConcurrentLogEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(concurrency_utils INTERFACE)

target_include_directories(concurrency_utils INTERFACE include)

target_link_libraries(concurrency_utils INTERFACE Threads::Threads)

add_executable(log_engine_daemon src/main.cpp)

target_link_libraries(log_engine_daemon PRIVATE concurrency_utils)
