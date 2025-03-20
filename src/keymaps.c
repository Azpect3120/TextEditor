#include "keymaps.h"
#include "rows.h"
#include <ncurses.h>
#include <ctype.h>
#include <string.h>

int editorProcessKeyPress(Editor *E, const int c) {

    // editorSetStatusMessage(E, "Key pressed: '%c' (%d)", (c == '\n') ? ' ' : c, c);


    switch (E->mode) {
        case NORMAL_MODE:
            return parse_command_normal(E, c);
        case INSERT_MODE:
            return parse_command_insert(E, c);
        case COMMAND_MODE:
            return parse_command_command(E, c);
    }

    return 0;

    // Ctrl-H is caught here
    if (c == KEY_BACKSPACE) {
        editorRemoveCharacter(E, E->cur_x, E->cur_y);
    } else if (c == KEY_DOWN) {
        moveCursor(E, DIRECTION_DOWN);
    } else if (c == KEY_UP) {
        moveCursor(E, DIRECTION_UP);
    } else if (c == KEY_LEFT) {
        moveCursor(E, DIRECTION_LEFT);
    } else if (c == KEY_RIGHT) {
        moveCursor(E, DIRECTION_RIGHT);
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

int parse_command_normal(Editor *E, int command) {

    if (command == 'i') {
        editorSwitchMode(E, INSERT_MODE);
    } else if (command == 'a') {
        moveCursor(E, DIRECTION_RIGHT);
        editorSwitchMode(E, INSERT_MODE);
    } else if (command == 'h') {
        moveCursor(E, DIRECTION_LEFT);
    } else if (command == 'j') {
        moveCursor(E, DIRECTION_DOWN);
    } else if (command == 'k') {
        moveCursor(E, DIRECTION_UP);
    } else if (command == 'l') {
        moveCursor(E, DIRECTION_RIGHT);
    } else if (command == 'x') {
        if ((E->cur_x) == E->row[E->cur_y].size) {
            editorRemoveCharacter(E, E->cur_x, E->cur_y);
        } else {
            editorRemoveCharacter(E, E->cur_x + 1, E->cur_y);
            moveCursor(E, DIRECTION_RIGHT);
        }
    // Keys between 1 and 26 SHOULD be control-characters
    } else if (iscntrl(command) && command >= 1 && command <= 26) {
        // Ctrl-C or Ctrl-Q: Quit
        if (command == 3 || command == 17) return 1;

        // Ctrl-S: Save
        if (command == 19) {
            editorSaveFile(E);
            return 0;
        }
    }

    return 0;
}

int parse_command_insert(Editor *E, int command) {

    if (command == KEY_BACKSPACE || command == 8) {
        editorRemoveCharacter(E, E->cur_x, E->cur_y);
    } else if (command == 27 || command == '\x1b') {
        editorSwitchMode(E, NORMAL_MODE);
        moveCursor(E, DIRECTION_LEFT);
    } else if (command == KEY_LEFT) {
        moveCursor(E, DIRECTION_LEFT);
    } else if (command == KEY_DOWN) {
        moveCursor(E, DIRECTION_DOWN);
    } else if (command == KEY_UP) {
        moveCursor(E, DIRECTION_UP);
    } else if (command == KEY_RIGHT) {
        moveCursor(E, DIRECTION_RIGHT);
    } else if (command == '\n' || command == KEY_ENTER || command == '\r') {
        editorInsertNewline(E);
    } else {
        editorInsertCharacter(E, E->cur_x, E->cur_y, (char) command);
    }

    return 0;
}
int parse_command_command(Editor *E, int command) { return 0; }

void editorSwitchMode(Editor *E, EditorMode new_mode) {
    E->mode = new_mode;
}

void moveCursor(Editor *E, CursorDirection dir) {
    switch (dir) {
        case DIRECTION_UP:
            if (E->cur_y > 0) {
                E->cur_y--;
                if (E->row != NULL && E->cur_x >= E->row[E->cur_y].size)
                    E->cur_x = E->row[E->cur_y].size;
            }
            break;
        case DIRECTION_DOWN:
            if (E->cur_y < E->num_rows - 1) {
                E->cur_y++;
                if (E->row != NULL && E->cur_x >= E->row[E->cur_y].size)
                    E->cur_x = E->row[E->cur_y].size;
            }
            break;
        case DIRECTION_LEFT:
            if (E->cur_x > 0) E->cur_x--;
            break;
        case DIRECTION_RIGHT:
            if (E->mode == INSERT_MODE)
                if (E->row != NULL && E->cur_x < E->row[E->cur_y].size) E->cur_x++;
            if (E->mode == NORMAL_MODE)
                if (E->row != NULL && E->cur_x < E->row[E->cur_y].size - 1) E->cur_x++;
            break;
    }
}
