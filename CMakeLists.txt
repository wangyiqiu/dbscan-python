cmake_minimum_required(VERSION 3.10)

project(dbscan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-std=c++17 -pthread -O3 -fPIC")

add_subdirectory(executable)

enable_testing()
add_subdirectory(test)
