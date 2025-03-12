//
// Created by azpect on 3/10/25.
//

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 * LINES stores the rows
 * COLS stores the cols
 * - These values update every time they are used!
 *
 * WMOVE(screen, x, y) MOVES the cursor
 */

/**
 * Editor row struct
 */
typedef struct erow {
    /**
     * @breif Length of the row in the x direction. Number of characters.
     * @note This value includes the '\0' terminator.
     */
    int size;

    /**
     * @brief Content stored in the row.
     * @note Length should NEVER exceed the 'size' value.
     */
    char *chars;
} erow;

/**
 * @breif Editor object.
 */
typedef struct Editor {
    /**
     * @breif Rows stored in the editor
     */
    erow *row;

    /**
     * @breif Number of rows in the file.
     * @note Not the size of the window, but the size of the content.
     */
    int num_rows;

    /**
     * @breif Number of rows that can be displayed on the screen.
     * @note This also serves as the capacity for the rows memory.
     */
    int screen_rows;

    /**
     * @breif Current x position of the cursor.
     * @note 0-indexed, where 0 is the left.
     */
    int cur_x;

    /**
     * @breif Current y position of the cursor.
     * @note 0-indexed, where 0 is the top.
     */
    int cur_y;

} Editor;


/**
 * Global editor state.
 */
Editor E;

/**
 * Write a 'row' to the buffer at 'pos.'
 * @param row Row to render
 * @param pos Position in the buffer
 */
void editorDrawRow(erow *row, int pos) {
    // TODO: Render the row, not just print
    mvwprintw(stdscr, pos, 0, "%s", row->chars);
}

/**
 * @brief Refresh the editor and draw each row.
 * @note This function draws '~' for unused lines.
 * @note This function also updates the editor size state.
 */
void editorRefresh() {
    // Update size state
    E.screen_rows = LINES;

    // Clear the screen before new render
    wclear(stdscr);

    // Loop the rows, and render each row
    int i;
    for (i = 0; i < E.num_rows; i++) {
        editorDrawRow(&E.row[i], i);
    }

    // Loop over unused rows
    for (i = E.num_rows; i < E.screen_rows; i++) {
        mvwprintw(stdscr, i, 0, "~");
    }

    // Move the cursor to the proper position defined in the state
    wmove(stdscr, E.cur_y, E.cur_x);
}

/**
 * @brief Inserts a row below 'pos' with the content [ s + '\0' ]
 * @param pos Position the cursor is at, 0-indexed
 * @param s String to append to the new row
 * @param len Size of the string to append
 * @note This function does NOT re-render, it just modifies the state.
 */
void editorInsertRowBelow(int pos, char *s, int len) {
    // Bounds check
    if (pos < 0 || pos > E.num_rows) return;

    // Reallocate enough memory for the new row
    E.row = realloc(E.row, sizeof(erow) * (E.num_rows + 1));

    // Move all memory (rows) after (below) the position back one to make room for the new row.
    memmove(&E.row[pos + 1], &E.row[pos], sizeof(erow) * (E.num_rows - pos));

    // Allocate memory for the new row, of size len + 1
    // Plus one is for the '\0'
    // Then, copy string into the new row and append '\0'
    E.row[pos].size = len;
    E.row[pos].chars = malloc(len + 1);
    memcpy(E.row[pos].chars, s, len);
    E.row[pos].chars[len] = '\0';

    // TODO: Create an updated render of the row

    // Increment the number of rows
    E.num_rows++;
}

/**
 * This was taken from kilo
 * TODO: Figure out what tf this does.
 */
void editorInsertNewline() {
    if (E.cur_x == 0) {
        editorInsertRowBelow(E.cur_y, "", 0);
    } else {
        erow *row = &E.row[E.cur_y];
        editorInsertRowBelow(E.cur_y + 1, &row->chars[E.cur_x], row->size - E.cur_x);
        row = &E.row[E.cur_y];
        row->size = E.cur_x;
        row->chars[row->size] = '\0';
    }

    E.cur_y++;
    E.cur_x = 0;
}

/**
 * @breif Insert a character c at (x, y)
 * @param x X position, location in the row
 * @param y Y position, row to insert into
 * @param c Character to insert
 */
void editorInsertCharacter(const int x, const int y, const char c) {
    // If we are on the last (or first, on open) create a line below
    if (y == E.num_rows) editorInsertRowBelow(y, "", 0);

    // Get the row we are working with
    erow *row = &E.row[y];

    // Increase the size
    row->size++;

    // Malloc one more character in memory
    row->chars = realloc(row->chars, sizeof(char) * row->size);

    // Move memory past x, over one
    memmove(&row->chars[x + 1], &row->chars[x], sizeof(char) * (row->size - x - 1));

    // Copy c into the x position
    row->chars[x] = c;

    // Append null-terminator
    // TODO: Abstract this into an update row function
    row->chars[row->size] = '\0';

    // Move the cursor one to the right
    E.cur_x++;
}

/**
 * @breif Insert a character at (x, y)
 * @param x X position, location in the row
 * @param y Y position, row to insert into
 */
void editorRemoveCharacter(const int x, const int y) {
    // Get the row we are working with
    erow *row = &E.row[y];

    // Bounds check
    if (x <= 0 || x > row->size) return;

    // TODO: IF x == 0, we need to delete the row

    // Move memory past x, over one
    memmove(&row->chars[x - 1], &row->chars[x], sizeof(char) * (row->size - x + 1));

    // Malloc one less character in memory
    if (x > 1) {
        row->chars = realloc(row->chars, sizeof(char) * (row->size - 1));
    } else {
        row->chars = realloc(row->chars, sizeof(char));
    }

    // Decrease the size
    row->size--;

    // Append null-terminator
    // TODO: Abstract this into an update row function
    row->chars[row->size] = '\0';

    // Move then cursor one to the left
    E.cur_x--;

}

int main () {
    E.row = NULL;
    E.num_rows = 0;
    E.screen_rows = LINES;

    // Initialize ncurses
    initscr();
    // start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);



    while (TRUE) {
        editorRefresh();

        int ch = getch();
        if (ch == KEY_BACKSPACE) {
            // TODO: Delete last character
            editorRemoveCharacter(E.cur_x, E.cur_y);
        } else if (ch == KEY_DOWN) {
            if (E.cur_y < E.num_rows) {
                E.cur_y++;
                if (E.cur_x >= E.row[E.cur_y].size) E.cur_x = E.row[E.cur_y].size;
            }
        } else if (ch == KEY_UP) {
            if (E.cur_y > 0) {
                E.cur_y--;
                if (E.cur_x >= E.row[E.cur_y].size) E.cur_x = E.row[E.cur_y].size;
            }
        } else if (ch == KEY_LEFT) {
            if (E.cur_x > 0) E.cur_x--;
        } else if (ch == KEY_RIGHT) {
            if (E.cur_x < E.row[E.cur_y].size) E.cur_x++;
        } else if (ch == '\n' || ch == KEY_ENTER || ch == '\r') {
            editorInsertNewline();
        } else {
            // editorAppendRow("ENTER", 0);
            editorInsertCharacter(E.cur_x, E.cur_y, (char)ch);
        }
    }

    // End ncurses mode
    endwin();
    return 0;
}
