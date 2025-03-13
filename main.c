#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
     * @breif Length of the raw characters in the x direction. Number of characters.
     * @note This value includes the '\0' terminator.
     */
    int size;

    /**
     * @breif Length of the render in the x direction. Number of characters.
     * @note This value includes the '\0' terminator.
     */
    int rsize;

    /**
     * @brief Content stored in the row.
     * @note Length should NEVER exceed the 'size' value.
     * @note These characters are used to generate the render, they should not be drawn directly.
     */
    char *chars;

    /**
     * @brief Content that should be drawn.
     * @note This value should be generated based on chars, before the screen is refreshed.
     */
    char *render;
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
 * @brief Global editor state.
 */
Editor E;

/**
 * @brief Render the row into the render field.
 * @param row Row to render
 * @note This function does not draw the row, it just generate the render.
 */
void editorRenderRow(erow *row) {
    // First, set size to the same.
    row->rsize = row->size;

    // Free the render
    if (row->render != NULL) free(row->render);

    // Allocate new memory for the render, plus one for the \0
    row->render = malloc(row->rsize + 1);

    // TODO: Actually generate a render, for now, just copy the memory
    memcpy(row->render, row->chars, sizeof(char) * row->rsize);

    // Fill the last empty byte with the terminator
    row->render[row->rsize] = '\0';
}

/**
 * Write a 'row' to the buffer at 'pos.'
 * @param row Row to render
 * @param pos Position in the buffer
 */
void editorDrawRow(erow *row, int pos) {
    mvwprintw(stdscr, pos, 0, "%s", row->render);
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
        // TODO: Do not rerender each row, just the ones that changed. So this will require a refactor of the insert/delete
        editorRenderRow(&E.row[i]);
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
 * @brief Free a row from memory.
 * @param row Target row to free from memory.
 */
void editorFreeRow(erow *row) {
    // TODO: Update this when more memory is added to the rows
    if (row->chars != NULL) free(row->chars);
    if (row->render != NULL) free(row->render);
}

/**
 * @breif Remove the line at position pos.
 * @param pos 0-indexed index to remove at
 * @note This function does NOT move the cursor.
 */
void editorRemoveRow(const int pos) {
    // Bounds check
    if (E.num_rows == 1 || pos >= E.num_rows) return;

    // Get the row we want to remove
    erow *row = &E.row[pos];
    editorFreeRow(row);

    // Move the memory on top of the old row
    memmove(&E.row[pos], &E.row[pos + 1], sizeof(erow) * (E.num_rows - pos - 1));

    // Decrease the row count
    E.num_rows--;
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
 * @note This function DOES not the cursor, once in the +x direction
 */
void editorInsertCharacter(const int x, const int y, const char c) {
    // Bounds check
    if (y < 0 || y >= E.num_rows) return;
    if (x < 0 || x > E.row[y].size) return;
    
    // If we are on the last (or first, on open) create a line below
    if (y == E.num_rows) editorInsertRowBelow(y, "", 0);

    // Get the row we are working with
    erow *row = &E.row[y];

    // Increase the size
    row->size++;

    // Malloc one more character in memory
    row->chars = realloc(row->chars, sizeof(char) * (row->size + 1));

    // Move memory past x, over one
    memmove(&row->chars[x + 1], &row->chars[x], sizeof(char) * (row->size - x - 1));

    // Copy c into the x position
    row->chars[x] = c;

    // Append null-terminator, since we increased the size of malloc by 1, we can append to the size.
    // TODO: Abstract this into an update row function
    row->chars[row->size] = '\0';

    // Move the cursor one to the right
    E.cur_x++;
}

/**
 * @brief Append content s to the end of row
 * @param row Row to append the content to
 * @param s Content to append to the row
 * @param len Size of the content to append
 * @note This function DOES move the cursor, in the x plane.
 */
void editorRowAppendStr(erow *row, const char *s, const int len) {
    // Update size
    // TODO: Why do I need this silly 0 check?
    row->size = (row->size == 0) ? len : row->size + len;

    // Reallocate memory to make space for new content, one extra byte for the \0
    row->chars = realloc(row->chars, sizeof(char) * (row->size + 1));

    // Copy the string into the row and append \0
    memcpy(&row->chars[row->size - len], s, len);
    row->chars[row->size] = '\0';

    // Set the cursor to the new position in the line
    E.cur_x = row->size - len;
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
    if (x < 0 || x > row->size) return;

    // If at pos 0 (start of line) we need to delete the line and move the content.
    if (x == 0) {
        if (row->size != 0 && y > 0) {
            editorRowAppendStr(&E.row[y - 1], row->chars, row->size);
        } else {
            if (E.row != NULL) E.cur_x = E.row[E.cur_y - 1].size;
        }
        editorRemoveRow(y);
        if (E.cur_y > 0) E.cur_y--;
        return;
    }

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

/**
 * Process the key presses
 * @param c Key pressed
 * @return 1 for exit, 0 for nothing
 */
int editorProcessKeyPress(const int c) {
    // Ctrl-H is caught here
    if (c == KEY_BACKSPACE) {
        editorRemoveCharacter(E.cur_x, E.cur_y);
    } else if (c == KEY_DOWN) {
        if (E.cur_y < E.num_rows - 1) {
            E.cur_y++;
            if (E.row != NULL && E.cur_x >= E.row[E.cur_y].size) E.cur_x = E.row[E.cur_y].size;
        }
    } else if (c == KEY_UP) {
        if (E.cur_y > 0) {
            E.cur_y--;
            if (E.row != NULL && E.cur_x >= E.row[E.cur_y].size) E.cur_x = E.row[E.cur_y].size;
        }
    } else if (c == KEY_LEFT) {
        if (E.cur_x > 0) E.cur_x--;
    } else if (c == KEY_RIGHT) {
        if (E.row != NULL && E.cur_x < E.row[E.cur_y].size) E.cur_x++;
    // Ctrl-M and Ctrl-J are caught here
    } else if (c == '\n' || c == KEY_ENTER || c == '\r') {
        editorInsertNewline();
        // Keys between 1 and 26 SHOULD be control-characters
    } else if (iscntrl(c) && c >= 1 && c <= 26) {
        // Ctrl-C or Ctrl-Q
        if (c == 3 ||c == 17) {
            return 1;
        }

        // TODO: Fix this with renders, right now, they are 2 characters
        // editorInsertCharacter(E.cur_x, E.cur_y, '^');
        editorInsertCharacter(E.cur_x, E.cur_y, (char) (c | 0x40)); // Set the 6th bit (0x40 = 64)
    } else {
        editorInsertCharacter(E.cur_x, E.cur_y, (char) c);
    }
    return 0;
}

int main () {
    E.row = NULL;
    E.num_rows = 0;
    E.screen_rows = LINES;


    // Initialize ncurses
    initscr();
    // TODO: Enable this when colors are enabled
    // start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    // TODO: Work on colors with the renderer
    // init_pair(1, COLOR_BLACK, COLOR_WHITE);


    editorInsertRowBelow(0, "", 0);


    int exit = 0;
    while (!exit) {
        editorRefresh();
        exit = editorProcessKeyPress(wgetch(stdscr));
    }

    // End ncurses mode
    endwin();
    return 0;
}
