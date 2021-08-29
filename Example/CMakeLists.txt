cmake_minimum_required(VERSION 3.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE readline)
pkg_check_modules(NCURSES ncurses)
if ("${READLINE_FOUND}" AND "${NCURSES_FOUND}")
	add_executable(shell shell.cpp)
	target_include_directories(shell PRIVATE ../HAL/include)
	target_link_libraries(shell router_hal ${READLINE_LIBRARIES} ${NCURSES_LIBRARIES})
endif()

add_executable(broadcaster broadcaster.cpp)
target_include_directories(broadcaster PRIVATE ../HAL/include)
target_link_libraries(broadcaster router_hal)

add_executable(capture capture.cpp)
target_include_directories(capture PRIVATE ../HAL/include)
target_link_libraries(capture router_hal)
