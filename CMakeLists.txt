cmake_minimum_required(VERSION 2.8.11)

project(extcpp)

install(FILES include/extcpp/safequeue.hpp DESTINATION include/extcpp)

add_subdirectory(test)
