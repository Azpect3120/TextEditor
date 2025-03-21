#ifndef SELECTION_H
#define SELECTION_H

#include <stdbool.h>

typedef enum {
    VISUAL_CHAR,
    VISUAL_BLOCK,
    VISUAL_LINE
} VisualModeType;

typedef struct {
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    bool active;
    VisualModeType type;
} VisualSelection;

#endif //SELECTION_H
