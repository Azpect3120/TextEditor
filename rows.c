#include "rows.h"
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

// TODO: Check for errors in allocation

void editorRemoveRow(Editor *E, const int pos) {
    // Bounds check
    if (E->num_rows == 1 || pos >= E->num_rows || pos == 0) return;

    // Get the row we want to remove
    erow *row = &E->row[pos];
    editorFreeRow(row);

    // Move the memory on top of the old row
    memmove(&E->row[pos], &E->row[pos + 1], sizeof(erow) * (E->num_rows - pos - 1));

    // Decrease the row count
    E->num_rows--;
}

void editorRenderRow(erow *row) {
    // Free the render
    // TODO: Might be better to reallocate here, to prevent some double freeing?
    if (row->render != NULL) free(row->render);

    // Calculate the number of tabs before allocation
    int tabs = 0;
    for (int i = 0; i < row->size; i++) if (row->chars[i] == '\t') tabs++;

    // Allocate new memory for the render, plus one for the '\0'
    row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

    int idx = 0;
    for (int i = 0; i < row->size; i++) {
        // Right now, the only difference between render and chars is the tabs
        if (row->chars[i] == '\t') {
            row->render[idx++] = ' ';
            while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[i];
        }
    }

    // Update render size and append terminator
    row->render[idx] = '\0';
    row->rsize = idx;
}

void editorDrawRow(erow *row, int pos) {
    mvwprintw(stdscr, pos, NUM_COL_SIZE, "%s", row->render);
}

void editorDrawRowNum(int cur_y, int pos) {
    int line_num = 0;
    char fmt[10];

    if (RELATIVE_NUM) {
        if (cur_y == pos) line_num = pos + 1;
        else if (cur_y > pos) line_num = cur_y - pos;
        else line_num = pos - cur_y;
    } else {
        line_num = pos + 1;
    }

    if (cur_y == pos) snprintf(fmt, sizeof(fmt), "%%%dd  ", NUM_COL_SIZE - 2);
    else snprintf(fmt, sizeof(fmt), "%%%dd ", NUM_COL_SIZE - 1);

    attron(COLOR_PAIR(2) | A_BOLD);
    mvwprintw(stdscr, pos, 0, fmt, line_num);
    attroff(COLOR_PAIR(2) | A_BOLD);
}

void editorFreeRow(erow *row) {
    if (row->chars != NULL) free(row->chars);
    // TODO: This was causing a double free when backspace pressed at x = 0
    // if (row->render != NULL) free(row->render);
}

void editorInsertRowBelow(Editor *E, int pos, char *s, int len) {
    // Bounds check
    if (pos < 0 || pos > E->num_rows) return;

    // Reallocate enough memory for the new row
    E->row = realloc(E->row, sizeof(erow) * (E->num_rows + 1));

    // Move all memory (rows) after (below) the position back one to make room for the new row.
    // We don't have to increment E.num_rows before this because num_rows stores 1 higher than the index (it's not 0 indexed)
    for (int i = E->num_rows; i > pos; i--) {
        int oldSize = E->row[i - 1].size;
        char *oldChars = E->row[i - 1].chars;

        E->row[i].chars = malloc(oldSize + 1);
        memcpy(E->row[i].chars, oldChars, oldSize);
        E->row[i].chars[oldSize] = '\0';
        E->row[i].size = oldSize;

        free(oldChars);
    }

    // Allocate memory for the new row, of size len + 1
    // Plus one is for the '\0'
    // Then, copy string into the new row and append '\0'
    E->row[pos].size = len;
    E->row[pos].chars = malloc(len + 1);
    memcpy(E->row[pos].chars, s, len);
    E->row[pos].chars[len] = '\0';

    // TODO: Create an updated render of the row

    // Increment the number of rows
    E->num_rows++;
}

void editorInsertNewline(Editor *E) {
    // Calculate previous line indentation
    int tabs = 0;
    if (E->cur_y > 0) {
        for (int i = 0; i < E->row[E->cur_y].size; i++)
            if (E->row[E->cur_y].chars[i] == '\t') tabs++;
            else break;
    }

    // Create newline string, +1 for null terminator
    char *indent = (char *)malloc((tabs + 1) * sizeof(char));
    for (int i = 0; i < tabs; i++) indent[i] = '\t';
    indent[tabs] = '\0';

    if (E->cur_x == 0) {
        editorInsertRowBelow(E, E->cur_y, indent, tabs);
    } else {
        erow *row = &E->row[E->cur_y];
        // Create the new string with the appended tabs, +1 for null terminator
        const int len = tabs + (row->size - E->cur_x);
        char *newChars = (char *)malloc((len + 1) * sizeof(char));

        // Concat the strings together
        strcpy(newChars, indent);
        strcat(newChars, &row->chars[E->cur_x]);
        newChars[len] = '\0';

        editorInsertRowBelow(E, E->cur_y + 1, newChars, len);
        row = &E->row[E->cur_y];
        row->size = E->cur_x;
    }

    E->cur_y++;
    E->cur_x = tabs;
    free(indent);
}

void rowAppendStr(Editor *E, erow *row, const char *s, const int len) {
    // Update size
    // TODO: Why do I need this silly 0 check?
    row->size = (row->size == 0) ? len : row->size + len;

    // Reallocate memory to make space for new content, one extra byte for the \0
    row->chars = realloc(row->chars, sizeof(char) * (row->size + 1));

    // Copy the string into the row and append \0
    memcpy(&row->chars[row->size - len], s, len);
    row->chars[row->size] = '\0';

    // Set the cursor to the new position in the line
    E->cur_x = row->size - len;
}

void editorInsertCharacter(Editor *E, const int x, const int y, const char c) {
    // Bounds check
    if (y < 0 || y >= E->num_rows) return;
    if (x < 0 || x > E->row[y].size) return;
    
    // If we are on the last (or first, on open), create a line below
    if (y == E->num_rows) editorInsertRowBelow(E, y, "", 0);

    // Get the row we are working with
    erow *row = &E->row[y];

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
    E->cur_x++;
}

void editorRemoveCharacter(Editor *E, const int x, const int y) {
    // Get the row we are working with
    erow *row = &E->row[y];

    // Bounds check
    if (x < 0 || x > row->size || (x == 0 && y == 0)) return;

    // If at pos 0 (start of line), we need to delete the line and move the content.
    if (x == 0) {
        if (row->size != 0 && y > 0) {
            rowAppendStr(E, &E->row[y - 1], row->chars, row->size);
        } else {
            if (E->row != NULL) E->cur_x = E->row[E->cur_y - 1].size;
        }
        editorRemoveRow(E, y);
        if (E->cur_y > 0) E->cur_y--;
        return;
    }

    // Move memory past x, over one
    memmove(&row->chars[x - 1], &row->chars[x], sizeof(char) * (row->size - x + 1));

    // We don't need to malloc less space, we just wait for the memory to be free when another change takes place

    // Decrease the size
    row->size--;

    // Append null-terminator
    // TODO: Abstract this into an update row function
    row->chars[row->size] = '\0';

    // Move then cursor one to the left
    E->cur_x--;
}

int editorRowGetRenderX(erow *row, int cur_x) {
    int rx = 0;
    for (int i = 0; i < cur_x; i++) {
        if (row->chars[i] == '\t') rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx + NUM_COL_SIZE;
}

