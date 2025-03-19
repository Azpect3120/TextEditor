#include "keymaps.h"
#include "rows.h"
#include <ncurses.h>
#include <ctype.h>

int editorProcessKeyPress(Editor *E, const int c) {
    editorSetStatusMessage(E, "Key pressed: '%c' (%d)", (c == '\n') ? ' ' : c, c);

    // Ctrl-H is caught here
    if (c == KEY_BACKSPACE) {
        editorRemoveCharacter(E, E->cur_x, E->cur_y);
    } else if (c == KEY_DOWN) {
        // TODO: This might be wrong to add the -1, it prevents going down a line into the status bar.
        if (E->cur_y < E->num_rows - 1) {
            E->cur_y++;
            if (E->row != NULL && E->cur_x >= E->row[E->cur_y].size)
                E->cur_x = E->row[E->cur_y].size;
        }
    } else if (c == KEY_UP) {
        if (E->cur_y > 0) {
            E->cur_y--;
            if (E->row != NULL && E->cur_x >= E->row[E->cur_y].size)
                E->cur_x = E->row[E->cur_y].size;
        }
    } else if (c == KEY_LEFT) {
        if (E->cur_x > 0) E->cur_x--;
    } else if (c == KEY_RIGHT) {
        if (E->row != NULL && E->cur_x < E->row[E->cur_y].size) E->cur_x++;
    // Ctrl-M and Ctrl-J are caught here
    } else if (c == '\n' || c == KEY_ENTER || c == '\r') {
        editorInsertNewline(E);
    // Keys between 1 and 26 SHOULD be control-characters
    } else if (iscntrl(c) && c >= 1 && c <= 26) {
        // Ctrl-C or Ctrl-Q: Quit
        if (c == 3 ||c == 17) return 1;

        // Ctrl-S: Save
        if (c == 19) {
            editorSaveFile(E);
            return 0;
        }

        // Catch Ctrl-I as tab for some reason...
        if (c == 9) {
            editorInsertCharacter(E, E->cur_x, E->cur_y, (char) c);
            return 0;
        }

        // TODO: Fix this with renders, right now, they are 2 characters
        // editorInsertCharacter(E->cur_x, E->cur_y, '^');
        editorInsertCharacter(E, E->cur_x, E->cur_y, (char) (c | 0x40)); // Set the 6th bit (0x40 = 64)
    } else {
        editorInsertCharacter(E, E->cur_x, E->cur_y, (char) c);
    }
    return 0;
}
