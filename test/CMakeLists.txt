cmake_minimum_required(VERSION 2.8.11)

enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test_safequeue test_safequeue.cpp)
target_include_directories(test_safequeue PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_safequeue ${GTEST_BOTH_LIBRARIES})
target_link_libraries(test_safequeue ${CMAKE_THREAD_LIBS_INIT})

add_test(AllTests test_safequeue)
