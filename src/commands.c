#include "commands.h"
#include <stdlib.h>

// ---- FILE I/O ----

void command_save(Editor *E, char *command) {
    editor_save_file(E);
}

void command_save_quit(Editor *E, char *command) {
    editor_save_file(E);
    editor_destroy(E);
    exit(0);
}

void command_quit(Editor *E, char *command) {
    editor_destroy(E);
    exit(0);
}

void command_force_quit(Editor *E, char *command) {
    // TODO: Implement force
    editor_destroy(E);
    exit(0);
}
