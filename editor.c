#include "editor.h"
#include "rows.h"
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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
        editorDrawRowNum(E->cur_y, i);
    }

    // Loop over unused rows except last one
    for (i = E->num_rows; i < E->screen_rows - 2; i++) {
        mvwprintw(stdscr, i, 0, "~");
    }

    // Check if the y is out of bounds
    if (E->cur_y >= E->num_rows) E->cur_y == E->num_rows - 1;

    // Calculate render cursor position
    E->ren_x = editorRowGetRenderX(&E->row[E->cur_y], E->cur_x);

    // Draw status bar and message bar
    editorDrawStatusBar(E);
    editorDrawMessage(E);

    if (E->cur_y >= E->screen_rows - 2) E->cur_y = E->screen_rows - 3;

    // Move the cursor to the proper position defined in the state
    wmove(stdscr, E->cur_y, E->ren_x);
}

void editorDrawStatusBar(Editor *E) {
    char *status_f = (char *)malloc((E->screen_cols + 1) * sizeof(char));
    char status_l[160], status_r[40];

    // Calculate bytes
    int bytes = 0;
    for (int i = 0; i < E->num_rows; i++) {
        bytes += E->row[i].size;
    }

    int len_l = snprintf(status_l, sizeof(status_l),
        "%.10s %.20s - %s",
        "INSERT",
        E->filename ? E->filename : "[No Name]",
        "(modified)"
        );
    int len_r = snprintf(status_r, sizeof(status_r),
        "%s | %db | %d:%d ",
        "no ft",
        bytes,
        E->cur_y + 1,
        E->ren_x + 1
        );

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
    mvwprintw(stdscr, E->screen_rows - 2, 0, "%s", status_f);
    attroff(COLOR_PAIR(1));

    free(status_f);
}

void editorDrawMessage(Editor *E) {
    // Clear the line before printing
    move(E->screen_rows - 1, 0);
    clrtoeol();
    if (E->message != (NULL) && time(NULL) - E->message_time < MESSAGE_TIMEOUT)
        mvwprintw(stdscr, E->screen_rows - 1, 0, "%s", E->message);
}

void editorSetStatusMessage(Editor *E, char *fmt, ...) {
    char *message = malloc(sizeof(char) * 80);

    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(message, sizeof(char) * 80, fmt, ap);
    va_end(ap);

    message[79] = '\0';

    E->message = realloc(E->message, sizeof(char) * (len + 1));
    strcpy(E->message, message);

    E->message_time = time(NULL);
    free(message);
}

void initEditor(Editor *E) {
    E->row = NULL;
    E->filename = NULL;
    E->num_rows = 0;
    E->screen_rows = LINES;
    E->screen_cols = COLS;

    // Initialize ncurses
    initscr();
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);


    // Set default colors
    assume_default_colors(COLOR_WHITE, COLOR_BLACK);
    use_default_colors();
};

void editorOpenFile(Editor *E, char *filename) {
    // Set the filename in the state
    free(E->filename);
    E->filename = strdup(filename);

    // Open the file
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        editorSetStatusMessage(E, "Failed to open %s", E->filename);
        return;
    }

    // Get the lines and add them to the editor
    char *line = NULL;
    size_t line_cap = 0;
    size_t line_len;
    while ((line_len = getline(&line, &line_cap, fp)) != -1) {
        // Remove the \n or \r from end of line
        while (line_len > 0 &&
              (line[line_len - 1] == '\n' ||
               line[line_len - 1] == '\r')) line_len--;

        editorInsertRowBelow(E, E->num_rows, line, line_len);
    }


    // Close the file and free memory
    fclose(fp);
    free(line);
}