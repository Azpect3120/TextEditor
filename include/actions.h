#ifndef ACTIONS_H
#define ACTIONS_H

#include "editor.h"

typedef enum {
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} direction;

// ---- UTIL ----
int is_whitespace(char c);

// ---- CURSOR ACTIONS ----

void action_move_cursor(Editor *E, direction dir);
void action_move_to_end_of_line(Editor *E);
void action_move_to_start_of_line(Editor *E);
void action_move_to_first_character(Editor *E);
void action_move_to_last_character(Editor *E);
void action_move_to_last_line(Editor *E);
void action_move_to_first_line(Editor *E);

// ---- NORMAL MODE ----
void action_insert_mode(Editor *E);
void action_insert_mode_start(Editor *E);
void action_insert_mode_append(Editor *E);
void action_insert_mode_append_end(Editor *E);
void action_insert_mode_below(Editor *E);
void action_insert_mode_above(Editor *E);
void action_move_start_line(Editor *E);
void action_move_end_line(Editor *E);
void action_move_first_char(Editor *E);
void action_move_next_word_start(Editor *E);
void action_move_prev_word_start(Editor *E);
void action_move_curr_word_end(Editor *E);
void action_move_left(Editor *E);
void action_move_down(Editor *E);
void action_move_up(Editor *E);
void action_move_right(Editor *E);
void action_delete_char(Editor *E);
void action_quit(Editor *E);
void action_save(Editor *E);

// ---- INSERT MODE ----
void action_normal_mode(Editor *E);
void action_backspace(Editor *E);
void action_enter(Editor *E);
void action_insert_character(Editor *E, char c);


#endif //ACTIONS_H
