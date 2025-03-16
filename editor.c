#include "editor.h"
#include "rows.h"
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>

void editorRefresh(Editor *E) {
    // Update size state
    E->screen_rows = LINES;
    E->screen_cols = COLS;

    // Clear the screen before new render
    wclear(stdscr);

    // Loop the rows, and render each row
    int i;
    for (i = 0; i < E->num_rows; i++) {
        // TODO: Do not rerender each row, just the ones that changed. So this will require a refactor of the insert/delete
        editorRenderRow(&E->row[i]);
        editorDrawRow(&E->row[i], i);
        editorDrawRowNum(i);
    }

    // Loop over unused rows except last one
    for (i = E->num_rows; i < E->screen_rows - 1; i++) {
        mvwprintw(stdscr, i, 0, "~");
    }

    // Check if the y is out of bounds
    if (E->cur_y >= E->num_rows) E->cur_y == E->num_rows - 1;

    // Calculate render cursor position
    E->ren_x = editorRowGetRenderX(&E->row[E->cur_y], E->cur_x);

    // Draw status bar
    editorDrawStatusBar(E);

    // Move the cursor to the proper position defined in the state
    wmove(stdscr, E->cur_y, E->ren_x);
}

void editorDrawStatusBar(Editor *E) {
    char *status_f = (char *)malloc((E->screen_cols + 1) * sizeof(char));
    char status_l[160], status_r[20];

    int len_l = snprintf(status_l, sizeof(status_l),
        "%.10s %.20s - %s %s",
        "INSERT",
        "[No Name]",
        "(modified)",
        (E->status_msg != NULL && time(NULL) - E->status_msg_time < MESSAGE_TIMEOUT) ?
            E->status_msg : ""
        );
    int len_r = snprintf(status_r, sizeof(status_r), "%s | %d:%d ", "no ft", E->cur_y + 1, E->ren_x + 1);

    if (len_l > E->screen_cols) len_l = E->screen_cols;
    strcpy(status_f, status_l);

    while (len_l < E->screen_cols) {
        if (E->screen_cols - len_l == len_r) {
            strcpy(&status_f[len_l], status_r);
            break;
        } else {
            status_f[len_l++] = ' ';
        }
    }
    status_f[E->screen_cols] = '\0';

    attron(COLOR_PAIR(1));
    mvwprintw(stdscr, E->screen_rows - 1, 0, "%s", status_f);
    attroff(COLOR_PAIR(1));

    free(status_f);
}

void editorSetStatusMessage(Editor *E, char *fmt, ...) {
    char *message = malloc(sizeof(char) * 80);

    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(message, sizeof(char) * 80, fmt, ap);
    va_end(ap);

    message[79] = '\0';

    E->status_msg = realloc(E->status_msg, sizeof(char) * (len + 1));
    strcpy(E->status_msg, message);

    E->status_msg_time = time(NULL);
    free(message);
}

void initEditor(Editor *E) {
    E->row = NULL;
    E->num_rows = 0;
    E->screen_rows = LINES;
    E->screen_cols = COLS;
};
