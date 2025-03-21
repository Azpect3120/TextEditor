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

void editor_refresh(Editor *E) {
    // Update size state
    E->screen_rows = LINES;
    E->screen_cols = COLS;

    // Clear the screen before new render
    wclear(stdscr);

    // Update scroll values
    editor_scroll(E);

    // Calculate the height of the screen, based on the rows
    int view_height = E->screen_rows - 2;

    for (int y = 0; y < view_height; y++) {
        int row_index = E->view_start + y;
        if (row_index >= 0 && row_index < E->num_rows) {
            editor_draw_row_num(E->cur_y - E->view_start,
                row_index - E->view_start,
                E->view_start);
            editor_draw_row(E, &E->row[row_index], y);
        } else {
            mvwprintw(stdscr, y, 0, "~");
        }
    }

    // Draw status bar and message bar
    editor_draw_status_bar(E);
    editor_draw_message(E);

    // Move the cursor to the proper position defined in the state
    wmove(stdscr, E->cur_y - E->view_start, E->ren_x);
}

void editor_scroll(Editor *E) {
    // Prevent the cursor from being on the last blank character in NORMAL MODE
    if (E->mode == NORMAL_MODE && E->cur_x == E->row[E->cur_y].size && E->cur_x > 0) E->cur_x--;

    // Calculate render cursor position
    E->ren_x = 0;
    if (E->cur_y < E->num_rows) E->ren_x = editor_row_get_render_x(&E->row[E->cur_y], E->cur_x);

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

void editor_draw_status_bar(Editor *E) {
    char *status_f = (char *)malloc((E->screen_cols + 1) * sizeof(char));
    char status_l[160], status_r[40];

    // Calculate bytes
    int bytes = 0;
    for (int i = 0; i < E->num_rows; i++) {
        bytes += E->row[i].size;
    }

    char *mode;
    switch (E->mode) {
        case NORMAL_MODE:
            mode = "NORMAL";
            break;
        case INSERT_MODE:
            mode = "INSERT";
            break;
        case VISUAL_MODE:
            mode = "VISUAL";
            break;
    }

    int len_l = snprintf(status_l, sizeof(status_l),
        " %.10s %.20s %s",
        mode,
        E->filename ? E->filename : "[No Name]",
        E->dirty != 0 ? "- (modified)" : ""
        );
    int len_r = snprintf(status_r, sizeof(status_r),
        "%s | %db | %d:%d ",
        E->filetype ? E->filetype : "no ft",
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

void editor_draw_message(Editor *E) {
    // Clear the line before printing
    move(E->screen_rows - 1, 0);
    clrtoeol();
    if (E->message != (NULL) && time(NULL) - E->message_time < MESSAGE_TIMEOUT)
        mvwprintw(stdscr, E->screen_rows - 1, 0, "%s", E->message);
}

void editor_set_status_message(Editor *E, char *fmt, ...) {
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

void init_editor(Editor *E) {
    // Initialize ncurses
    initscr();
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    E->row = NULL;
    E->filename = NULL;
    E->dirty = 0;
    E->num_rows = 0;
    E->cur_x = 0;
    E->cur_y = 0;
    E->view_start = 0;
    E->screen_rows = LINES;
    E->screen_cols = COLS;
    E->mode = NORMAL_MODE;
    E->selection = malloc(sizeof(VisualSelection));

    // Set esc to be handled instantly
    ESCDELAY = 0;

    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);


    // Set default colors
    assume_default_colors(COLOR_WHITE, COLOR_BLACK);
    use_default_colors();
}

void editor_destroy(Editor *E) {
    endwin();

    // TODO: Clear any memory allocated in the editor
    free(E->selection);
};

void editor_open_file(Editor *E, char *filename) {
    // Set the filename in the state
    free(E->filename);
    E->filename = strdup(filename);

    // Detect and update the filetype
    editor_detect_file_type(E);

    // Open the file
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        // Append row to the first line to allow for typing, same as in main
        editor_insert_row_below(E, 0, "", 0);
        editor_set_status_message(E, "%s does not exist, it will be created on save.", E->filename);
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

        editor_insert_row_below(E, E->num_rows, line, line_len);
    }


    // Close the file and free memory
    fclose(fp);
    free(line);
}

void editor_save_file(Editor *E) {
    if (E->filename == NULL) {
        char *filename = editor_prompt(E, "Enter a filename: %s", NULL);
        if (filename == NULL) {
            editor_set_status_message(E, "Failed to save. Invalid file name.");
            return;
        }
        E->filename = filename;
    }

    // Detect and update the filetype
    editor_detect_file_type(E);

    // Convert the content to a string
    int len;
    char *buf = editor_content_to_string(E, &len);

    int fd = open(E->filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        // Truncate file to the content size, the bytes will be written over, but
        // excess bytes will not, so we remove them here.
        if (ftruncate(fd, len) != 1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                editor_set_status_message(E, "%d bytes written to %s", len, E->filename);
                E->dirty = 0;
                return;
            }
        }
    }

    // Catch error and free buffer
    free(buf);
    editor_set_status_message(E, "Failed to save: %s", strerror(errno));
}

void editor_detect_file_type(Editor *E) {
    if (E->filename == NULL) return;
    char *dot = strrchr(E->filename, '.');

    // Unknown filetype: missing .ext
    if (dot == NULL) return;
    E->filetype = ++dot;
}

char *editor_content_to_string(Editor *E, int *buf_len) {
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

char *editor_prompt(Editor *E, char *prompt, void (*callback)(char *, int)) {
    // TODO: Callback is ignored, implement it for searching
    // Create input buffer
    size_t buf_size = 128;
    char *buf = malloc(buf_size);

    size_t buf_len = 0;
    buf[0] = '\0';

    // Hold delay value, and set it to 0 for this function
    int delay = ESCDELAY;
    ESCDELAY = 0;

    while (true) {
        editor_set_status_message(E, prompt, buf);
        editor_refresh(E);

        int c = wgetch(stdscr);
        if (c == KEY_BACKSPACE) {
            if (buf_len > 0) buf[--buf_len] = '\0';
        } else if (c == '\n' || c == KEY_ENTER || c == '\r') {
            editor_set_status_message(E, "");
            ESCDELAY = delay;
            return buf;
        // Catch ESC: There doesn't seem to be an escape key
        } else if (c == 27 || c == '\x1b') {
            editor_set_status_message(E, "");
            ESCDELAY = delay;
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

bool editor_inside_selection(Editor *E, int x, int y) {
    VisualSelection *s = E->selection;

    // Catch this, put it in the logic though
    if (!s->active) return false;


    // START is ABOVE END but not equal
    if (s->start_y > s->end_y) {
        // ON STARTING LINE
        if (y == s->start_y) return (s->start_x <= x);

        // ON ENDING LINE
        if (y == s->end_y) return (s->end_x >= x);

        // LINES BETWEEN THE END POINTS
        return (s->start_y >= y && y >= s->end_y);
    }

    // END is ABOVE START but not equal
    if (s->start_y < s->end_y) {
        // ON STARTING LINE
        if (y == s->start_y) return (s->start_x <= x);

        // ON ENDING LINE
        if (y == s->end_y) return (s->end_x >= x);

        // LINES BETWEEN THE END POINTS
        return (s->end_y >= y && y >= s->start_y);
    }


    // START is END
    return (s->start_x <= x && x <= s->end_x && (s->start_y == y && y == s->end_y));
}