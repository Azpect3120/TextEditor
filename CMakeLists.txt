cmake_minimum_required(VERSION 3.30)
project(TextEditor C)

set(CMAKE_C_STANDARD 99)

find_package(Curses REQUIRED)

if (CURSES_FOUND)
    include_directories(${CURSES_INCLUDE_DIRS})
    add_executable(TextEditor main.c)
    target_link_libraries(TextEditor ${CURSES_LIBRARIES})
else()
    message(FATAL_ERROR "ncurses not found")
endif()
