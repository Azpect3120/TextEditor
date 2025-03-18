#ifndef EDITOR_H
#define EDITOR_H

#define TAB_STOP 4
#define MESSAGE_TIMEOUT 5
#define NUM_COL_SIZE 5
#define RELATIVE_NUM true
#define SCROLL_OFF 8

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
     * @brief The start of the view being rendered.
     * @note Gemini finally spat this solution out.
     */
    int view_start;

    /**
     * @breif Message to display in the message bar.
     */
    char *message;

    /**
     * @brief Stores the time of the last update to the message.
     * @note Used to update the message on refresh.
     */
    int message_time;

    /**
     * @brief Name of the file that is open, if none exists, the user will
     * be prompt to enter one when saving.
     */
    char *filename;
} Editor;

/**
 * @brief Refresh the editor and draw each row.
 * @param E Editor state
 * @note This function draws '~' for unused lines.
 * @note This function also updates the editor size state.
 */
void editorRefresh(Editor *E);

/**
 * @brief Handles the editors scroll functionality. As well as x-position.
 * @param E Editor state
 * @note Scroll-off constant is used to determine the scroll.
 * @note This function DOES NOT move the cursor, just updates the state.
 * @note Gemini wrote this, so if changes need to be made, good luck.
 */
void editorScroll(Editor *E);

/**
 * @brief Initialize the editor state object.
 * @param E Editor state
 */
void initEditor(Editor *E);

/**
 * @brief Draw the status bar with the content pre-defined. No message here.
 * @param E Editor state
 * @note This will be called on each render.
 * @note This function will move the cursor, so it should be moved back after
 * this function is called.
 */
void editorDrawStatusBar(Editor *E);

/**
 * @brief Draw the message bar with the content in the editor state.
 * @param E Editor state
 * @note This will be called on each render.
 * @note This function will move the cursor, so it should be moved back after
 * this function is called.
 */
void editorDrawMessage(Editor *E);

/**
 * @brief Update the message in the status bar.
 * @param E Editor state.
 * @param fmt Format specifier.
 * @param ... Optional arguments for the fmt.
 */
void editorSetStatusMessage(Editor *E, char *fmt, ...);

// TODO: MOVE THESE TO A FILES.C file

/**
 * @brief Open a file and load it's content into the editor.
 * @param E Editor state
 * @param filename Name of the file to open
 */
void editorOpenFile(Editor *E, char *filename);

/**
 * @brief Save the content in the editor to the file that is opened.
 * @param E Editor state
 * TODO: Handle NULL filename, for now, it skips.
 */
void editorSaveFile(Editor *E);

char *editorContentToString(Editor *E, int *buf_len);


#endif //EDITOR_H
