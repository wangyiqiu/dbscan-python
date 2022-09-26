cmake_minimum_required(VERSION 3.10)

project(dbscan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-std=c++17 -pthread -O3 -fPIC")

add_executable(dbscan main.cpp ../src/capi.cpp)
target_include_directories(dbscan PRIVATE ../src/)
