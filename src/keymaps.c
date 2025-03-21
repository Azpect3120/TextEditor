#include "keymaps.h"
#include "rows.h"
#include "actions.h"
#include <ncurses.h>
#include <ctype.h>
#include <string.h>

KeyMap normal_mode_keymaps[] = {
    {'i', action_insert_mode},
    {'I', action_insert_mode_start},
    {'a', action_insert_mode_append},
    {'A', action_insert_mode_append_end},
    {'o', action_insert_mode_below},
    {'O', action_insert_mode_above},
    {'w', action_move_next_word_start}, // TODO: differentiate W and w
    {'b', action_move_prev_word_start}, // TODO: differentiate B and b
    {'e', action_move_curr_word_end},   // TODO: differentiate E and e
    {'0', action_move_start_line},
    {'$', action_move_end_line},
    {'_', action_move_first_char},
    {'h', action_move_left},
    {'j', action_move_down},
    {'k', action_move_up},
    {'l', action_move_right},
    {'x', action_delete_char},
    // {3, action_quit},  // Ctrl-C
    // {17, action_quit}, // Ctrl-Q
    {19, action_save}, // Ctrl-S
    {KEY_ENTER, action_move_down}, // Enter
    {'\r', action_move_down},      // Enter
    {'\n', action_move_down},      // Enter
    {KEY_BACKSPACE, action_move_left},
    {8, action_move_left},      // BACKSPACE
    {':', action_command_mode},

    {0, NULL} // Null terminator: ALL MAPS MUST BE ABOVE THIS
};

KeyMap insert_mode_keymaps[] = {
    {'\x1b', action_normal_mode},  // ESC
    {27, action_normal_mode},      // ESC
    {KEY_BACKSPACE, action_backspace},
    {8, action_backspace},      // BACKSPACE
    {KEY_ENTER, action_enter},  // Enter
    {'\r', action_enter},       // Enter
    {'\n', action_enter},       // Enter
    {23, action_delete_last_word}, // Ctrl-W
    {KEY_LEFT, action_move_left},
    {KEY_RIGHT, action_move_right},
    {KEY_UP, action_move_up},
    {KEY_DOWN, action_move_down},

    {0, NULL} // Null terminator: ALL MAPS MUST BE ABOVE THIS
};

void editor_process_key_press(Editor *E, const int c) {

    // editor_set_status_message(E, "Key pressed: '%c' (%d)", (c == '\n') ? ' ' : c, c);
    switch (E->mode) {
        case NORMAL_MODE:
            execute_command_normal(E, c);
            break;
        case INSERT_MODE:
            execute_command_insert(E, c);
            break;
        case COMMAND_MODE:
            break;
    }
}

int execute_command_normal(Editor *E, const int command) {
    for (int i = 0; normal_mode_keymaps[i].action != NULL; i++) {
        if (command == normal_mode_keymaps[i].key) {
            normal_mode_keymaps[i].action(E);
            return 0;
        }
    }
    return  -1;
}

int execute_command_insert(Editor *E, const int command) {
    for (int i = 0; insert_mode_keymaps[i].action != NULL; i++) {
        if (command == insert_mode_keymaps[i].key) {
            insert_mode_keymaps[i].action(E);
            return 0;
        }
    }
    action_insert_character(E, (char) command);

    return  0;
}

