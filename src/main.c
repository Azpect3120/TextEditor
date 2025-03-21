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
    init_editor(&E);

    if (argc >= 2) {
        editor_open_file(&E, argv[1]);
    } else {
        // Append row to the first line to allow for typing
        editor_insert_row_below(&E, 0, "", 0);
    }

    while (true) {
        editor_refresh(&E);
        editor_process_key_press(&E, wgetch(stdscr));
    }
}
