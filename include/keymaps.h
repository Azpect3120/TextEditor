#ifndef KEYMAPS_H
#define KEYMAPS_H

#include "editor.h"

/**
 * Keymap struct which allows for easy mapping of keys
 */
typedef struct {
    int key; // TODO: Might need to make this a string to handle multiple presses? Or maybe store the previous presses in a buffer?
    void (*action)(Editor *E);
} KeyMap;

/**
 * Command mapping struct which allows for easy mapping of keys
 * @note The functions must also take the command as an argument,
 * for parsing purposes.
 */
typedef struct {
    char *command;
    void (*action)(Editor *E, char *command);
} CmdMap;

/**
 * Process the key presses
 * @param E Editor state
 * @param c Key pressed
 */
void editor_process_key_press(Editor *E, const int c);

/**
 * Execute a command in normal mode. The mappings are pre-defined.
 * @param E Editor state
 * @param command Key press or command to execute
 * @return Status code
 * @note Return codes: 0 = success; -1 = unknown command;
 */
int execute_command_normal(Editor *E, int command);

int execute_command_insert(Editor *E, int command);

int execute_command_visual(Editor *E, int command);

int execute_command(Editor *E, char *command);

#endif //KEYMAPS_H

/**
 * Add a value to the editor state called STACK to store previous key pressed
 * Add a value to the editor state called REPEAT to store the repeat value, and just loop it.
 * Use repeat to execute the commands many times.
 */