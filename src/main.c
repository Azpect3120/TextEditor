#include <ncurses.h>
#include "editor.h"
#include "keymaps.h"

#include "rows.h"

/*
 *
 * LINES stores the rows
 * COLS stores the cols
 * - These values update every time they are used!
 *
 * WMOVE(screen, x, y) MOVES the cursor
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

    while (true) {
        editorRefresh(&E);
        editor_process_key_press(&E, wgetch(stdscr));
    }
}
