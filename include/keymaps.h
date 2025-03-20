#ifndef KEYMAPS_H
#define KEYMAPS_H

#include "editor.h"

typedef enum {
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} CursorDirection;

/**
 * Process the key presses
 * @param E Editor state
 * @param c Key pressed
 * @return 1 for exit, 0 for nothing
 */
int editorProcessKeyPress(Editor *E, const int c);

// TODO: These should be able to handle multiple key presses, but for now just one
int parse_command_normal(Editor *E, int command);
int parse_command_insert(Editor *E, int command);
int parse_command_command(Editor *E, int command);

void editorSwitchMode(Editor *E, EditorMode new_mode);

/**
 * @brief Move the cursor.
 * @param E Editor state
 * @param dir Direction to move the cursor
 * @note This function handles bounds checking, do not worry about checking
 * before calling this function.
 */
void moveCursor(Editor *E, CursorDirection dir);

#endif //KEYMAPS_H

/**
 * CREATE AN ACTIONS FILE WITH ACTIONS LIKE 'x' and 'd' and END OF LINE and abstract everything there.
 * Add a value to the editor state called STACK to store previous key pressed
 * Add a value to the editor state called REPEAT to store the repeat value, and just loop it.
 * Use repeat to execute the commands many times.
 */