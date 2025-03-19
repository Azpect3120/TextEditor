#include "editor.h"
#include "rows.h"

#include <errno.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>

void editorRefresh(Editor *E) {
    // Update size state
    E->screen_rows = LINES;
    E->screen_cols = COLS;

    // Clear the screen before new render
    wclear(stdscr);

    // Update scroll values
    editorScroll(E);

    // Calculate the height of the screen, based on the rows
    int view_height = E->screen_rows - 2;

    for (int y = 0; y < view_height; y++) {
        int row_index = E->view_start + y;
        if (row_index >= 0 && row_index < E->num_rows) {
            editorDrawRowNum(E->cur_y - E->view_start,
                row_index - E->view_start,
                E->view_start);
            editorDrawRow(&E->row[row_index], y);
        } else {
            mvwprintw(stdscr, y, 0, "~");
        }
    }

    // Draw status bar and message bar
    editorDrawStatusBar(E);
    editorDrawMessage(E);

    // Move the cursor to the proper position defined in the state
    wmove(stdscr, E->cur_y - E->view_start, E->ren_x);
}

void editorScroll(Editor *E) {
    // Calculate render cursor position
    E->ren_x = 0;
    if (E->cur_y < E->num_rows) E->ren_x = editorRowGetRenderX(&E->row[E->cur_y], E->cur_x);

    // Subtract status and message bars
    int view_height = E->screen_rows - 2;

    // Ensure the cursor is within view with offset
    if (E->cur_y < E->view_start + SCROLL_OFF) {
        E->view_start = E->cur_y - SCROLL_OFF;
        if (E->view_start < 0) E->view_start = 0;
    } else if (E->cur_y >= E->view_start + view_height - SCROLL_OFF) {
        E->view_start = E->cur_y - view_height + 1 + SCROLL_OFF;
    }

    // Boundary check
    if (E->view_start < 0) E->view_start = 0;
    if (E->view_start + view_height > E->num_rows) {
        E->view_start = E->num_rows - view_height;
        if (E->view_start < 0) E->view_start = 0;
    }
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
    // Initialize ncurses
    initscr();
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    E->row = NULL;
    E->filename = NULL;
    E->num_rows = 0;
    E->cur_x = 0;
    E->cur_y = 0;
    E->view_start = 0;
    E->screen_rows = LINES;
    E->screen_cols = COLS;


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

void editorSaveFile(Editor *E) {
    if (E->filename == NULL) {
        char *filename = editorPrompt(E, "Enter a filename: %s", NULL);
        if (filename == NULL) {
            editorSetStatusMessage(E, "Cannot save a null file silly goose! (TODO: Fix this)");
            return;
        }
        E->filename = filename;
    }

    // Convert the content to a string
    int len;
    char *buf = editorContentToString(E, &len);

    int fd = open(E->filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        // Truncate file to the content size, the bytes will be written over, but
        // excess bytes will not, so we remove them here.
        if (ftruncate(fd, len) != 1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                editorSetStatusMessage(E, "%d bytes written to %s", len, E->filename);
                return;
            }
        }
    }

    // Catch error and free buffer
    free(buf);
    editorSetStatusMessage(E, "Failed to save: %s", strerror(errno));
}

char *editorContentToString(Editor *E, int *buf_len) {
    // Compute total length and update passed value
    int tot_len = 0;
    for (int i = 0; i < E->num_rows; i++)
        tot_len += E->row[i].size + 1;
    *buf_len = tot_len;

    // Generate the final string
    char *buf = malloc(tot_len);
    char *p = buf;
    for (int i = 0; i < E->num_rows; i++) {
        memcpy(p, E->row[i].chars, E->row[i].size);
        p += E->row[i].size;
        *p = '\n';
        p++;
    }

    return buf;
}

char *editorPrompt(Editor *E, char *prompt, void (*callback)(char *, int)) {
    // TODO: Callback is ignored, implement it for searching
    // Create input buffer
    size_t buf_size = 128;
    char *buf = malloc(buf_size);

    size_t buf_len = 0;
    buf[0] = '\0';

    while (true) {
        editorSetStatusMessage(E, prompt, buf);
        editorRefresh(E);

        int c = wgetch(stdscr);
        if (c == KEY_BACKSPACE) {
            if (buf_len > 0) buf[--buf_len] = '\0';
        } else if (c == '\n' || c == KEY_ENTER || c == '\r') {
            editorSetStatusMessage(E, "");
            return buf;
        // Catch ESC: There doesn't seem to be an escape key
        } else if (c == 27 || c == '\x1b') {
            editorSetStatusMessage(E, "");
            return NULL;
        } else if (!iscntrl(c) && c > 26) {
            if (buf_len == buf_size - 1) {
                buf_size *= 2;
                buf = realloc(buf, buf_size);
            }

            buf[buf_len++] = (char) c;
            buf[buf_len] = '\0';
        }
    }
}