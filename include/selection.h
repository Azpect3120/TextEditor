#ifndef SELECTION_H
#define SELECTION_H

#include <stdbool.h>

typedef struct {
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    bool active;
} VisualSelection;

#endif //SELECTION_H
