cmake_minimum_required(VERSION 3.30)
project(TextEditor C)

set(CMAKE_C_STANDARD 99)

find_package(Curses REQUIRED)

if (CURSES_FOUND)
    include_directories(${CURSES_INCLUDE_DIRS} include)
    add_executable(TextEditor
            src/main.c
            src/rows.c
            src/editor.c
            src/keymaps.c
            src/actions.c
            include/actions.h
    )
    target_link_libraries(TextEditor ${CURSES_LIBRARIES})
else()
    message(FATAL_ERROR "ncurses not found")
endif()
