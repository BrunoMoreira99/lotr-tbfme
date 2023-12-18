#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console_utils.h"
#include "ui_io_utils.h"


void printCenteredText(const char* str) {
    printf("\x1B[%dG%s", (getConsoleWidth() - (int)strlen(str)) / 2, str);
}

/**
 * Prints a full-width, single line, box with a title inside.
 * @param title The text to print inside the box.
 */
void drawFullWidthBoxTitle(const char* title) {
    const uint16_t margin = 2;
    const uint16_t consoleWidth = getConsoleWidth();
    const uint16_t cursorTop = getCursorPosition() >> 16 & 0xFFFF;
    setCursorHorizontalPosition(margin);
    printf("┏");
    for (uint16_t i = margin; i < consoleWidth - margin; i++) {
        printf("━");
    }
    printf("┓");
    setCursorVerticalHorizontalPosition(cursorTop + 1, margin); printf("┃  %s", title);
    setCursorHorizontalPosition(consoleWidth - margin + 1); printf("┃");
    setCursorVerticalHorizontalPosition(cursorTop + 2, margin); printf("┗");
    for (uint16_t i = margin; i < consoleWidth - margin ; i++) {
        printf("━");
    }
    printf("┛\n");
}

/**
 * Prints a box with rounded corners at the current position with the given width and height.
 * @param width The width of the box.
 * @param height The height of the box.
 */
void drawRoundedBox(const uint16_t width, const uint16_t height) {
    const uint16_t cursorLeft = getCursorPosition() & 0xFFFF;
    printf("╭");
    for (uint16_t i = 0; i < width - 2; i++) {
        printf("─");
    }
    printf("╮\n");
    for (uint16_t i = 0; i < height - 2; i++) {
        printf("\x1B[%dG│\x1B[%dG│\n", cursorLeft, cursorLeft + width - 1);
    }
    printf("\x1B[%dG╰", cursorLeft);
    for (uint16_t i = 0; i < width - 2; i++) {
        printf("─");
    }
    printf("╯\n");
}

/**
 * Prints a box with thicker lines at the current position with the given width and height.
 * @param width The width of the box.
 * @param height The height of the box.
 */
void drawBoldBox(const uint16_t width, const uint16_t height) {
    const uint16_t cursorLeft = getCursorPosition() & 0xFFFF;
    printf("┏");
    for (uint16_t i = 0; i < width - 2; i++) {
        printf("━");
    }
    printf("┓\n");
    for (uint16_t i = 0; i < height - 2; i++) {
        printf("\x1B[%dG┃\x1B[%dG┃\n", cursorLeft, cursorLeft + width - 1);
    }
    printf("\x1B[%dG┗", cursorLeft);
    for (uint16_t i = 0; i < width - 2; i++) {
        printf("━");
    }
    printf("┛\n");
}

/**
 * Prints a box at the current position with the given width and height.
 * @param width The width of the box.
 * @param height The height of the box.
 */
void drawBox(const uint16_t width, const uint16_t height) {
    const uint16_t cursorLeft = getCursorPosition() & 0xFFFF;
    printf("┌");
    for (uint16_t i = 0; i < width - 2; i++) {
        printf("─");
    }
    printf("┐\n");
    for (uint16_t i = 0; i < height - 2; i++) {
        printf("\x1B[%dG│\x1B[%dG│\n", cursorLeft, cursorLeft + width - 1);
    }
    printf("\x1B[%dG└", cursorLeft);
    for (uint16_t i = 0; i < width - 2; i++) {
        printf("─");
    }
    printf("┘\n");
}

void drawBoxWithTitleAndFooter(const char* title, const char* footer, const uint16_t width, const uint16_t height, void (*boxDrawingFunc)(const uint16_t, const uint16_t)) {
    const uint32_t cursorPos = getCursorPosition();
    const uint16_t cursorTop = cursorPos >> 16 & 0xFFFF, cursorLeft = cursorPos & 0xFFFF;
    boxDrawingFunc(width, height);
    if (title != NULL) {
        setCursorVerticalHorizontalPosition(cursorTop, cursorLeft + 2);
        printf(" %s ", title);
    }
    if (footer != NULL) {
        setCursorVerticalHorizontalPosition(cursorTop + height - 1, cursorLeft + 2);
        printf(" %s ", footer);
    }
}

/**
 * Reads a line from stdin and stores it in the string pointed to by `str`.
 * The function reads at most `maxSize` characters from the input stream, including the terminating null byte ('\0').
 * If `str` is NULL, the function returns NULL.
 * If an error occurs, the function returns NULL.
 * Otherwise, the function returns `str`.
 *
 * @param str Pointer to the buffer where the line read is stored.
 * @param maxSize Maximum number of characters to be read (including the final null-character).
 * @return Pointer to the buffer where the line read is stored.
 */
char* _readLine(char* str, const size_t maxSize) {
    if (!str) return NULL;
    if (!fgets(str, (int)maxSize, stdin)) return NULL;
    const size_t i = strcspn(str, "\n");
    if (i < maxSize - 1) {
        str[i] = '\0';
    } else {
        // Clear the input buffer if the line is too long.
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    return str;
}

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
char* readLine(char* str, const size_t maxSize, const bool allowEmpty) {
    if (!str) return NULL;
    if (allowEmpty) {
        _readLine(str, maxSize);
    } else {
        const uint32_t initialCursorPosition = getCursorPosition();
        while (1) {
            _readLine(str, maxSize);
            if (str[0] == ' ') {
                setCursorPosition(initialCursorPosition);
                for (size_t i = 1; i < maxSize; i++) {
                    printf(" ");
                    if (str[i] == '\0') break;
                }
                setCursorPosition(initialCursorPosition);
                continue;
            }
            if (str[0] == '\0') {
                setCursorPosition(initialCursorPosition);
                continue;
            }
            break;
        }
    }
    return str;
}

/**
 * Reads a line from a file and returns it as a string (char*).
 *
 * @param fp A pointer to the file to read from.
 * @return Pointer to the buffer where the line read is stored.
 *         The string must be freed by the caller.
 *         Returns NULL if the file pointer is NULL or if an error occurs.
 */
char* fReadLine(FILE* fp) {
    if (fp == NULL) return NULL;
    size_t bufSize = 120;
    char *buf = malloc(bufSize * sizeof(char));
    if (buf == NULL) return NULL;
    size_t currentPos = 0;
    while (true) {
        const int currentCh = fgetc(fp);
        if (currentCh == EOF || currentCh == '\n') break;
        buf[currentPos] = (char)currentCh;
        if (++currentPos == bufSize) {
            bufSize *= 2;
            char *tempBuf = buf;
            buf = realloc(buf, bufSize * sizeof(char));
            if (buf == NULL) return tempBuf;
        }
    }
    buf[currentPos] = '\0';
    return buf;
}
