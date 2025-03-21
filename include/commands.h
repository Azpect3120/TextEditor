#ifndef COMAMNDS_H
#define COMAMNDS_H

#include "editor.h"

// ---- FILE I/O ----

void command_save(Editor *E, char *command);
void command_save_quit(Editor *E, char *command);
void command_quit(Editor *E, char *command);
void command_force_quit(Editor *E, char *command);

#endif //COMAMNDS_H
