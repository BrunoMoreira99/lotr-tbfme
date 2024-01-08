#pragma once
#include <stdint.h>

#define MENU_BACK INT32_MAX
#define MENU_FOOTER_GO_BACK "Press [ESC] to Go Back"
#define MENU_FOOTER_CANCEL "Press [ESC] to Cancel"

typedef enum {
    KEY_ENTER = 13,
    KEY_ESC = 27,
    KEY_SPACE = 32,
    KEY_UP = 72,
    KEY_LEFT = 75,
    KEY_RIGHT = 77,
    KEY_DOWN = 80,
    KEY_W = 119,
    KEY_A = 97,
    KEY_S = 115,
    KEY_D = 100
} KeyCode;

typedef enum {
    BLACK = 0x000000,
    WHITE = 0xFFFFFF,
    DARK_GRAY = 0x666666,
    RED = 0xE74856,
    YELLOW = 0xE5C07B,
    GREEN = 0x98C379
} Color;

typedef struct {
    const char* text;
    const bool disabled;
    const int16_t rowOffset;
} MenuOption;

typedef struct {
    const char* text;
    const uint16_t castarCoinCost;
    const bool disabled;
    const uint8_t row;
    const uint16_t consoleColumn;
} ActionMenuOption;

/** Prompts the user to resize the console window to a supported dimension.
 * 
 * @param textA A string containing the first line of the prompt message.
 *              If NULL, the default message "Window size below minimum requirements!" will be used.
 * @param textB A string containing the second line of the prompt message.
 *              If NULL, the default message "Increase the window size to proceed." will be used.
 * @param minWidth The minimum width of the console window.
 * @param minHeight The minimum height of the console window.
 * @return True if the console was resized and graphics should be redrawn, False if no action is needed.
 */
bool enforceConsoleResize(const char* textA, const char* textB, uint16_t minWidth, uint16_t minHeight);

/**
 * Modifies the brightness of a given color by the specified amount.
 * 
 * @param color The original color in 24-bit RGB format.
 * @param brightnessModifier The brightness modifier, a value lesser than 1.0 will darken the color,
 *                           a value greater than 1.0 will brighten the color.
 * @return The modified color.
 */
uint32_t modifyColorBrightness(uint32_t color, float brightnessModifier);

/**
 * Prints a given string centered on the console.
 * 
 * @param str The string to be printed.
 */
void printCenteredText(const char* str);

/**
 * Prints a full-width, single line, box with a title inside.
 * 
 * @param title The text to print inside the box.
 */
void drawFullWidthBoxTitle(const char* title);

/**
 * Prints a box with rounded corners at the current position with the given width and height.
 * 
 * @param width The width of the box.
 * @param height The height of the box.
 */
void drawRoundedBox(uint16_t width, uint16_t height);

/**
 * Prints a box with thicker lines at the current position with the given width and height.
 * 
 * @param width The width of the box.
 * @param height The height of the box.
 */
void drawBoldBox(uint16_t width, uint16_t height);

/**
 * Prints a box at the current position with the given width and height.
 * 
 * @param width The width of the box.
 * @param height The height of the box.
 */
void drawBox(uint16_t width, uint16_t height);

/**
 * Prints a box at the current position with the given title, footer, width, and height.
 * 
 * @param title The title of the box.
 * @param footer The footer of the box.
 * @param width The width of the box.
 * @param height The height of the box.
 * @param boxDrawingFunc The box drawing function to call.
 */
void drawBoxWithTitleAndFooter(
    const char* title,
    const char* footer,
    uint16_t width,
    uint16_t height,
    void (*boxDrawingFunc)(uint16_t, uint16_t)
);

/**
 * Reads a line from stdin and stores it in the string pointed to by `str`.
 * The function reads at most `maxSize` characters from the input stream, including the terminating null byte ('\0').
 * If `str` is NULL, the function returns NULL.
 * If an error occurs, the function returns NULL.
 * Otherwise, the function returns `str`.
 *
 * If `allowEmpty` is true, the function reads the line even if it is empty.
 * Otherwise, the function reads the line only if it is not empty and does not start with a space character.
 * If the line is empty or starts with a space character, the function clears the input buffer and reads the line again.
 *
 * @param str Pointer to the buffer where the line read is stored.
 * @param maxSize Maximum number of characters to be read (including the final null-character).
 * @param allowEmpty Flag indicating whether to allow empty lines.
 * @return Pointer to the buffer where the line read is stored.
 */
char* readLine(char* str, size_t maxSize, bool allowEmpty);

/**
 * Reads a line from a file and returns it as a string (char*).
 *
 * @param fp A pointer to the file to read from.
 * @return Pointer to the buffer where the line read is stored.
 *         The string must be freed by the caller.
 *         Returns NULL if the file pointer is NULL or if an error occurs.
 */
char* fReadLine(FILE* fp);
