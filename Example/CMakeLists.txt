cmake_minimum_required(VERSION 2.8)

add_executable(shell shell.cpp)
target_include_directories(shell PRIVATE ../HAL/include)
target_link_libraries(shell router_hal readline ncurses)

add_executable(broadcaster broadcaster.cpp)
target_include_directories(broadcaster PRIVATE ../HAL/include)
target_link_libraries(broadcaster router_hal)

add_executable(capture capture.cpp)
target_include_directories(capture PRIVATE ../HAL/include)
target_link_libraries(capture router_hal)