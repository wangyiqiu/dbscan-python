cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

# check whether googletest is locally installed, if not download and fetch
message(STATUS "--------------- GoogleTest -------------")
find_package(GTest CONFIG)
if(NOT GTest_FOUND)
    # new way of including googletest
    # Download and unpack googletest at configure time
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
            ${CMAKE_BINARY_DIR}/googletest-build
            EXCLUDE_FROM_ALL)
    set(GTest_LIBRARIES "gtest")
else()
    message(STATUS "using locally installed GoogleTest")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(GTest_LIBRARIES GTest::gtest)
endif()

include(CTest)

project(sourceFiles)
add_library(sourceFiles INTERFACE)
target_include_directories(sourceFiles INTERFACE ../src/)

add_executable(grid_test grid_test.cpp)
target_link_libraries(grid_test PRIVATE
  sourceFiles
  ${GTest_LIBRARIES})
add_test(NAME grid_test COMMAND grid_test)

add_executable(dbscan_test dbscan_test.cpp ../src/capi.cpp)
target_link_libraries(dbscan_test PRIVATE
  sourceFiles
  ${GTest_LIBRARIES})
add_test(NAME dbscan_test COMMAND dbscan_test)

# add_executable(dbscan_test dbscan_test.cpp)
# target_link_libraries(dbscan_test PRIVATE
#   gridLib
#   dbscanLib
#   ${GTest_LIBRARIES})
# add_test(NAME dbscan_test COMMAND dbscan_test)
