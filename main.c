#include <ncurses.h>
#include "editor.h"
#include "keymaps.h"
#include <stdlib.h>

#include "rows.h"

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
int main (int argc, char *argv[]) {
    Editor E;
    initEditor(&E);

    if (argc >= 2) {
        editorOpenFile(&E, argv[1]);
    } else {
        // Append row to the first line to allow for typing
        editorInsertRowBelow(&E, 0, "", 0);
    }

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
