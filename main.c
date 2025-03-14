#include <ncurses.h>
#include "editor.h"
#include "rows.h"
#include "keymaps.h"

/*
 *
 * LINES stores the rows
 * COLS stores the cols
 * - These values update every time they are used!
 *
 * WMOVE(screen, x, y) MOVES the cursor
 */

/**
 * @brief Global editor state.
 */
int main () {
    Editor E;
    initEditor(&E);

    // Initialize ncurses
    initscr();
    // TODO: Enable this when colors are enabled
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    // TODO: Work on colors with the renderer
    init_pair(1, COLOR_BLACK, COLOR_WHITE);


    // Set default colors
    assume_default_colors(COLOR_WHITE, COLOR_BLACK);
    use_default_colors();

    editorInsertRowBelow(&E, 0, "", 0);

    int exit = 0;
    while (!exit) {
        editorRefresh(&E);
        exit = editorProcessKeyPress(&E, wgetch(stdscr));
    }


    // TODO: Clear editor memory
    // End ncurses mode
    endwin();
    return 0;
}
