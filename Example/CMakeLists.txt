cmake_minimum_required(VERSION 2.8)

add_executable(shell shell.cpp)
target_include_directories(shell PRIVATE ../HAL/include)
target_link_libraries(shell router_hal readline ncurses)

add_executable(broadcaster broadcaster.cpp)
target_include_directories(broadcaster PRIVATE ../HAL/include)
target_link_libraries(broadcaster router_hal)

if(${BACKEND} STREQUAL LINUX)
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/network-principle-labs/routing_table/target/release/librouting_table.a
    COMMAND
        make
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/network-principle-labs/routing_table
)

add_custom_target(
    librouting_table
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/network-principle-labs/routing_table/target/release/librouting_table.a
)

file(GLOB FILES ./network-principle-labs/forwarder/*.c ./network-principle-labs/ripd/*.c*)
add_executable(forwarder ${FILES})
add_dependencies(forwarder librouting_table)
target_include_directories(forwarder PRIVATE ../HAL/include)
target_link_libraries(forwarder router_hal readline ncurses pthread ${CMAKE_CURRENT_SOURCE_DIR}/network-principle-labs/routing_table/target/release/librouting_table.a dl)
endif()