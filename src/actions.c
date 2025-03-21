#include "actions.h"

int is_whitespace(const char c) {
    return (c == ' ' || c == '\t' || c == '\r');
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
