#include "actions.h"
#include "rows.h"
#include <stddef.h>
#include <stdbool.h>


// ---- UTIL ----

int is_whitespace(const char c) {
    return (c == ' ' || c == '\t' || c == '\r');
}


// ---- CURSOR ACTIONS ----

void action_move_cursor(Editor *E, const direction dir) {
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

void action_move_to_end_of_line(Editor *E) {
    E->cur_x = E->row[E->cur_y].size;
}

void action_move_to_start_of_line(Editor *E) {
    E->cur_x = 0;
}

void action_move_to_first_character(Editor *E) {
    erow *row = &E->row[E->cur_y];

    int i;
    for (i = 0; i < row->size; i++)
        if (!is_whitespace(row->chars[i])) break;
    E->cur_x = i;
}

void action_move_to_last_character(Editor *E) {
    erow *row = &E->row[E->cur_y];

    int i;
    for (i = row->size - 1; i > 0; i--)
        if (!is_whitespace(row->chars[i])) break;
    E->cur_x = i + 1;
}

void action_move_to_last_line(Editor *E){}

void action_move_to_first_line(Editor *E){}


// ---- NORMAL MODE ----

void action_insert_mode(Editor *E) {
    E->mode = INSERT_MODE;
};

void action_insert_mode_start(Editor *E) {
    E->mode = INSERT_MODE;
    action_move_to_first_character(E);
};

void action_insert_mode_append(Editor *E) {
    E->mode = INSERT_MODE;
    action_move_cursor(E, DIRECTION_RIGHT);
};

void action_insert_mode_append_end(Editor *E) {
    E->mode = INSERT_MODE;
    action_move_to_end_of_line(E);
};

void action_move_start_line(Editor *E) {
    action_move_to_start_of_line(E);
};

void action_move_end_line(Editor *E) {
    action_move_to_end_of_line(E);
};

void action_move_first_char(Editor *E) {
    action_move_to_first_character(E);
};

void action_move_left(Editor *E) {
    action_move_cursor(E, DIRECTION_LEFT);
};

void action_move_down(Editor *E) {
    action_move_cursor(E, DIRECTION_DOWN);
};

void action_move_up(Editor *E) {
    action_move_cursor(E, DIRECTION_UP);
};

void action_move_right(Editor *E) {
    action_move_cursor(E, DIRECTION_RIGHT);
};

void action_delete_char(Editor *E) {
    if ((E->cur_x) == E->row[E->cur_y].size) {
        editor_remove_character(E, E->cur_x, E->cur_y);
    } else {
        editor_remove_character(E, E->cur_x + 1, E->cur_y);
        action_move_cursor(E, DIRECTION_RIGHT);
    }
};

void action_quit(Editor *E) {
    editor_destroy(E);
    exit(0);
};

void action_save(Editor *E) {
    editor_save_file(E);
};

void action_move_next_word_start(Editor *E) {
    erow *row = &E->row[E->cur_y];

    int i;
    bool space_found = false;
    for (i = E->cur_x; i < row->size; i++) {
        if (row->chars[i] == ' ') space_found = true;
        if (space_found && row->chars[i] != ' ') break;
    }

    // TODO: Move to next lines when at the end
    // If we didn't find another word, move to the next line
    // if (!char_found && !space_found && E->cur_y < E->num_rows) {
    //     erow *next_row = &E->row[E->cur_y + 1];
    //     for (i = 0; i < next_row->size; i++) {
    //         if (row->chars[i] == ' ') space_found = true;
    //         if (space_found && row->chars[i] != ' ') break;
    //     }
    //     E->cur_y++;
    // }

    E->cur_x = i;
}

void action_move_prev_word_start(Editor *E) {
    erow *row = &E->row[E->cur_y];

    int i;
    bool space_found = false;
    bool chars_found = false;
    for (i = E->cur_x; i > 0; i--) {
        if (row->chars[i] == ' ') space_found = true;
        if (space_found && row->chars[i] != ' ') chars_found = true;
        if (chars_found && space_found && row->chars[i] == ' ') break;
    }

    E->cur_x = ++i;

}

void action_move_curr_word_end(Editor *E) {

}

// ---- INSERT MORE ----

void action_normal_mode(Editor *E) {
    E->mode = NORMAL_MODE;
    action_move_cursor(E, DIRECTION_LEFT);
}

void action_backspace(Editor *E) {
    editor_remove_character(E, E->cur_x, E->cur_y);
}

void action_enter(Editor *E) {
    editor_insert_newline(E);
}

void action_insert_character(Editor *E, const char c) {
    editor_insert_character(E, E->cur_x, E->cur_y, c);
}
