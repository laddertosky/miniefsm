cmake_minimum_required(VERSION 3.20)
project(miniefsm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Header-only library target
add_library(miniefsm INTERFACE)
target_include_directories(miniefsm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_subdirectory(tests)
