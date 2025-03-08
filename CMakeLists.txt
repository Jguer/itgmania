cmake_minimum_required(VERSION 3.20)

project(StepMania)

# Set C++ standard to at least C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(StepmaniaCore.cmake)

# The external libraries need to be included.
add_subdirectory(extern)

# The internal libraries and eventual executable to be used.
add_subdirectory(src)
