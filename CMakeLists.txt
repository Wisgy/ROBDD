cmake_minimum_required(VERSION 3.10)

project(ROBDD)

enable_testing()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(DCMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(src)
add_subdirectory(test)
target_include_directories(ROBDD PRIVATE include)