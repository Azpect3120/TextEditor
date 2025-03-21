#ifndef EDITOR_H
#define EDITOR_H

#include "selection.h"
#include <stdbool.h>

#define TAB_STOP 4
#define MESSAGE_TIMEOUT 5
#define NUM_COL_SIZE 5
#define RELATIVE_NUM true
#define SCROLL_OFF 8

typedef enum {
    NORMAL_MODE,
    INSERT_MODE,
    VISUAL_MODE
} EditorMode;

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
     * Content stored in the row.
     * @note Length should NEVER exceed the 'size' value.
     * @note These characters are used to generate the render, they should not be drawn directly.
     */
    char *chars;

    /**
     * Content that should be drawn.
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
     * Current x position of the cursor in the render.
     * @note 0-indexed, where 0 is the left.
     */
    int ren_x;

    /**
     * The start of the view being rendered.
     * @note Gemini finally spat this solution out.
     */
    int view_start;

    /**
     * @breif Message to display in the message bar.
     */
    char *message;

    /**
     * Stores the time of the last update to the message.
     * @note Used to update the message on refresh.
     */
    int message_time;

    /**
     * Name of the file that is open, if none exists, the user will
     * be prompt to enter one when saving.
     */
    char *filename;

    /**
     * Type of the file in the editor.
     * @note Stored as the file extension excluding the period.
     */
    char *filetype;

    /**
     * @breif Stores a value of how "dirty" the file is. The number of changes
     * made to the file.
     * @note Any value over 0 indicates a file has been changed.
     */
    int dirty;

    /**
     * Mode the editor is in.
     * @note Mode will determine the way in which commands are parsed.
     */
    EditorMode mode;

    VisualSelection *selection;
} Editor;

/**
 * Refresh the editor and draw each row.
 * @param E Editor state
 * @note This function draws '~' for unused lines.
 * @note This function also updates the editor size state.
 */
void editor_refresh(Editor *E);

/**
 * Handles the editors scroll functionality. As well as x-position.
 * @param E Editor state
 * @note Scroll-off constant is used to determine the scroll.
 * @note This function DOES NOT move the cursor, just updates the state.
 * @note Gemini wrote this, so if changes need to be made, good luck.
 */
void editor_scroll(Editor *E);

/**
 * Initialize the editor state object.
 * @param E Editor state
 */
void init_editor(Editor *E);

void editor_destroy(Editor *E);

/**
 * Draw the status bar with the content pre-defined. No message here.
 * @param E Editor state
 * @note This will be called on each render.
 * @note This function will move the cursor, so it should be moved back after
 * this function is called.
 */
void editor_draw_status_bar(Editor *E);

/**
 * Draw the message bar with the content in the editor state.
 * @param E Editor state
 * @note This will be called on each render.
 * @note This function will move the cursor, so it should be moved back after
 * this function is called.
 */
void editor_draw_message(Editor *E);

/**
 * Update the message in the status bar.
 * @param E Editor state.
 * @param fmt Format specifier.
 * @param ... Optional arguments for the fmt.
 */
void editor_set_status_message(Editor *E, char *fmt, ...);

// TODO: MOVE THESE TO A FILES.C file

/**
 * Open a file and load it's content into the editor.
 * @param E Editor state
 * @param filename Name of the file to open
 */
void editor_open_file(Editor *E, char *filename);

/**
 * Save the content in the editor to the file that is opened.
 * @param E Editor state
 * @note This function will set the dirty value to 0, indicating all changes
 * are saved.
 */
void editor_save_file(Editor *E);

/**
 * Detect the filetype of the filename stored in the state.
 * @param E Editor state
 */
void editor_detect_file_type(Editor *E);

/**
 * Convert the content in the rows into a string. The size will be
 * updated and set in the buf_len parameter.
 * @param E Editor state
 * @param buf_len Length of the content (will be updated)
 * @return Content as a \\n split string
 */
char *editor_content_to_string(Editor *E, int *buf_len);

/**
 * Prompt the user to fill out a prompt.
 * This will take over control of the keymaps and send them all into this function.
 * If ESC is pressed at any point during the prompt, nothing will be returned.
 * @param E Editor state
 * @param prompt Prompt string
 * @param callback Callback to call when submitted
 * @return The content that was provided by the user
 * @note TODO: Callback is not handled, just a parameter for now
 * @note A string format specifier is expected to be in the prompt string.
 */
char *editor_prompt(Editor *E, char *prompt, void (*callback)(char *,int));

/**
 * Determines if the position provided by x and y is inside the current selection.
 * This will be used for highlighting.
 * @param E Editor state
 * @param x X position (0-indexed)
 * @param y Y position (0-indexed)
 */
bool editor_inside_selection(Editor *E, int x, int y);

#endif //EDITOR_H
