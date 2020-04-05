cmake_minimum_required(VERSION 2.8)

if(${BACKEND} STREQUAL EMCC)
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --pre-js ${CMAKE_CURRENT_SOURCE_DIR}/../HAL/src/emcc/preamble.js --js-library ${CMAKE_CURRENT_SOURCE_DIR}/../HAL/src/emcc/library.js -s ASYNCIFY -s 'ASYNCIFY_IMPORTS=[\"read_packet\"]'")
endif()

if(NOT ${BACKEND} STREQUAL EMCC)
add_executable(shell shell.cpp)
target_include_directories(shell PRIVATE ../HAL/include)
target_link_libraries(shell router_hal readline ncurses)
endif()

add_executable(broadcaster broadcaster.cpp)
target_include_directories(broadcaster PRIVATE ../HAL/include)
target_link_libraries(broadcaster router_hal)

add_executable(capture capture.cpp)
target_include_directories(capture PRIVATE ../HAL/include)
target_link_libraries(capture router_hal)
