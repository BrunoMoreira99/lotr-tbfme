#pragma once
#include <stdint.h>
#include <stdbool.h>

bool setupConsole(const char* windowTitle);

void setConsoleWindowTitle(const char* str);

void setForegroundColor(const uint32_t color);
void setBackgroundColor(const uint32_t color);

static inline void clearConsole(void) {
    printf("\x1B[H\x1B[2J\x1B[3J");
}

static inline void clearLine(const uint16_t line) {
    printf("\x1B[%dH\x1B[2K", line);
}

static inline void clearCurrentLine(void) {
    printf("\x1B[2K");
}

static inline void clearCurrentLineFromCursorForward(void) {
    printf("\x1B[0K");
}

static inline void clearCurrentLineFromCursorBackward(void) {
    printf("\x1B[1K");
}

static inline void resetForegroundColor(void) {
    printf("\x1B[39m");
}

static inline void resetBackgroundColor(void) {
    printf("\x1B[49m");
}

static inline void resetTextDecoration(void) {
    printf("\x1B[0m");
}

static inline void showCursor(void) {
    printf("\x1B[?25h");
}

static inline void hideCursor(void) {
    printf("\x1B[?25l");
}

static inline void enableBlinking(void) {
    printf("\x1B[5m");
}

static inline void disableBlinking(void) {
    printf("\x1B[25m");
}

uint32_t getCursorPosition(void);

static inline void setCursorPosition(const uint32_t position) {
    // Row, Column
    printf("\x1B[%d;%dH", position >> 16 & 0xFFFF, position & 0xFFFF);
}

static inline void setCursorVerticalHorizontalPosition(const uint16_t row, const uint16_t column) {
    printf("\x1B[%d;%dH", row, column);
}

static inline void setCursorVerticalPosition(const uint16_t row) {
    printf("\x1B[%dH", row);
}

static inline void setCursorHorizontalPosition(const uint16_t column) {
    printf("\x1B[%dG", column);
}

static inline void resetCursorPosition(void) {
    printf("\x1B[H");
}

uint32_t getConsoleDimensions(void);
uint16_t getConsoleWidth(void);
uint16_t getConsoleHeight(void);
