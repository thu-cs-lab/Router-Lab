cmake_minimum_required(VERSION 2.8)
add_executable(shell shell.cpp)
target_include_directories(shell PRIVATE ../HAL/include)
target_link_libraries(shell router_hal)
