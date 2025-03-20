#ifndef KEYMAPS_H
#define KEYMAPS_H

#include "editor.h"

/**
 * Process the key presses
 * @param E Editor state
 * @param c Key pressed
 * @return 1 for exit, 0 for nothing
 */
int editorProcessKeyPress(Editor *E, const int c);

#endif //KEYMAPS_H
