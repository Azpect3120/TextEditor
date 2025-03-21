#ifndef KEYMAPS_H
#define KEYMAPS_H

#include "editor.h"

/**
* @brief Keymap struct which allows for easy mapping of keys
*/
typedef struct {
    int key; // TODO: Might need to make this a string to handle multiple presses? Or maybe store the previous presses in a buffer?
    void (*action)(Editor *E);
} KeyMap;

/**
 * @brief Process the key presses
 * @param E Editor state
 * @param c Key pressed
 */
void editor_process_key_press(Editor *E, const int c);

/**
 * @brief Execute a command in normal mode. The mappings are pre-defined.
 * @param E Editor state
 * @param command Key press or command to execute
 * @return Status code
 * @note Return codes: 0 = success; -1 = unknown command;
 */
int execute_command_normal(Editor *E, int command);

int execute_command_insert(Editor *E, int command);

#endif //KEYMAPS_H

/**
 * CREATE AN ACTIONS FILE WITH ACTIONS LIKE 'x' and 'd' and END OF LINE and abstract everything there.
 * Add a value to the editor state called STACK to store previous key pressed
 * Add a value to the editor state called REPEAT to store the repeat value, and just loop it.
 * Use repeat to execute the commands many times.
 */