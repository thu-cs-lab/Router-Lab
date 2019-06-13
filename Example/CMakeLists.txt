cmake_minimum_required(VERSION 2.8)
add_executable(shell shell.cpp)
target_include_directories(shell PRIVATE ../HAL/include)
target_link_libraries(shell router_hal readline ncurses)

file(GLOB FILES ./network-principle-labs/forwarder/*.c)
add_executable(forwarder ${FILES})
target_include_directories(forwarder PRIVATE ../HAL/include)
target_link_libraries(forwarder router_hal readline ncurses pthread ${CMAKE_CURRENT_SOURCE_DIR}/network-principle-labs/routing_table/target/release/librouting_table.a dl)