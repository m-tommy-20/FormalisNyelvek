cmake_minimum_required(VERSION 3.10)

project(project CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directory for project headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add your own problems here
add_executable(project main.cpp problems/sum.cpp problems/dfa.cpp)
