#ifndef EDITOR_H
#define EDITOR_H

#define TAB_STOP 4
#define MESSAGE_TIMEOUT 5

/**
 * Editor row struct
 */
typedef struct erow {
    /**
     * @breif Length of the raw characters in the x direction. Number of characters.
     * @note This value includes the '\0' terminator.
     */
    int size;

    /**
     * @breif Length of the render in the x direction. Number of characters.
     * @note This value includes the '\0' terminator.
     */
    int rsize;

    /**
     * @brief Content stored in the row.
     * @note Length should NEVER exceed the 'size' value.
     * @note These characters are used to generate the render, they should not be drawn directly.
     */
    char *chars;

    /**
     * @brief Content that should be drawn.
     * @note This value should be generated based on chars, before the screen is refreshed.
     */
    char *render;
} erow;

/**
 * @breif Editor object.
 */
typedef struct Editor {
    /**
     * @breif Rows stored in the editor
     */
    erow *row;

    /**
     * @breif Number of rows in the file.
     * @note Not the size of the window, but the size of the content.
     */
    int num_rows;

    /**
     * @breif Number of rows that can be displayed on the screen.
     * @note This also serves as the capacity for the rows memory.
     */
    int screen_rows;

    /**
     * @breif Number of columns that can be displayed on the screen.
     */
    int screen_cols;

    /**
     * @breif Current x position of the cursor.
     * @note 0-indexed, where 0 is the left.
     */
    int cur_x;

    /**
     * @breif Current y position of the cursor.
     * @note 0-indexed, where 0 is the top.
     */
    int cur_y;

    /**
     * @brief Current x position of the cursor in the render.
     * @note 0-indexed, where 0 is the left.
     */
    int ren_x;

    /**
     * @breif Message to display in the status bar.
     */
    char *status_msg;

    /**
     * @brief Stores the time of the last update to the status message.
     * @note Used to update the status message on refresh.
     */
    int status_msg_time;
} Editor;

/**
 * @brief Refresh the editor and draw each row.
 * @param E Editor state
 * @note This function draws '~' for unused lines.
 * @note This function also updates the editor size state.
 */
void editorRefresh(Editor *E);

/**
 * Initialize the editor state object.
 * @param E Editor state
 */
void initEditor(Editor *E);

/**
 * Draw the status bar with the message stored in the editor state.
 * @param E Editor state
 */
void editorDrawStatusBar(Editor *E);

/**
 * Update the message in the status bar.
 * @param E Editor state.
 * @param fmt Format specifier.
 * @param ... Optional arguments for the fmt.
 */
void editorSetStatusMessage(Editor *E, char *fmt, ...);

#endif //EDITOR_H
