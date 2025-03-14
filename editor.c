#include "editor.h"
#include "rows.h"
#include <ncurses.h>

void editorRefresh(Editor *E) {
    // Update size state
    E->screen_rows = LINES;

    // Clear the screen before new render
    wclear(stdscr);

    // Loop the rows, and render each row
    int i;
    for (i = 0; i < E->num_rows; i++) {
        // TODO: Do not rerender each row, just the ones that changed. So this will require a refactor of the insert/delete
        editorRenderRow(&E->row[i]);
        editorDrawRow(&E->row[i], i);
    }

    // Loop over unused rows
    for (i = E->num_rows; i < E->screen_rows; i++) {
        mvwprintw(stdscr, i, 0, "~");
    }

    // Move the cursor to the proper position defined in the state
    wmove(stdscr, E->cur_y, E->cur_x);
}

void initEditor(Editor *E) {
    E->row = NULL;
    E->num_rows = 0;
    E->screen_rows = LINES;
};
