#ifndef ROWS_H
#define ROWS_H

#include "editor.h"
#include <stdlib.h>

/**
 * @breif Remove the line at position pos.
 * @param E Editor state
 * @param pos 0-indexed index to remove at
 * @note This function does NOT move the cursor.
 */
void editorRemoveRow(Editor *E, const int pos);

/**
 * @brief Render the row into the render field.
 * @param row Row to render
 * @note This function does not draw the row, it just generate the render.
 */
void editorRenderRow(erow *row);

/**
 * Write a 'row' to the buffer at 'pos.'
 * @param row Row to render
 * @param pos Position in the buffer
 * @note The position will be offset by the NUM_COL_SIZE
 */
void editorDrawRow(erow *row, int pos);

/**
 * Draws the row number to the row at pos.
 * @param cur_y Current y position
 * @param pos Position in the buffer
 * @param offset Offset to add to each line number, allows for scrolling
 * @note Pos should be the index of the row, one should be added for the print-out.
 */
void editorDrawRowNum(int cur_y, int pos, int offset);

/**
 * @brief Free a row from memory.
 * @param row Target row to free from memory.
 */
void editorFreeRow(erow *row);

/**
 * @brief Inserts a row below 'pos' with the content [ s + '\0' ]
 * @param E Editor state
 * @param pos Position the cursor is at, 0-indexed
 * @param s String to append to the new row
 * @param len Size of the string to append
 * @note This function does NOT re-render, it just modifies the state.
 */
void editorInsertRowBelow(Editor *E, int pos, char *s, size_t len);

/**
 * This was taken from kilo
 * TODO: Figure out what tf this does.
 * @note This function will increment the dirty counter.
 */
void editorInsertNewline(Editor *E);

/**
 * @brief Append content s to the end of row
 * @param E Editor state
 * @param row Row to append the content to
 * @param s Content to append to the row
 * @param len Size of the content to append
 * @note This function DOES move the cursor, in the x plane.
 */
void rowAppendStr(Editor *E, erow *row, const char *s, const int len);

/**
 * @breif Insert a character c at (x, y)
 * @param E Editor state
 * @param x X position, location in the row
 * @param y Y position, row to insert into
 * @param c Character to insert
 * @note This function DOES move the cursor, in the +x direction.
 * @note This function will increment the dirty counter.
 */
void editorInsertCharacter(Editor *E, const int x, const int y, const char c);

/**
 * @breif Insert a character at (x, y)
 * @param E Editor state
 * @param x X position, location in the row
 * @param y Y position, row to insert into
 * @note This function will increment the dirty counter.
 */
void editorRemoveCharacter(Editor *E, const int x, const int y);

/**
 * Compute the position of the cursor in the render based on the current position.
 * @param row Row to generate render position for.
 * @param cur_x Current position of the cursor in the x direction.
 * @return ren_x position
 */
int editorRowGetRenderX(erow *row, int cur_x);


#endif //ROWS_H