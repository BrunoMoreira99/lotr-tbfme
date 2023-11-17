#pragma once

typedef enum {
    WHITE = 0xFFFFFF,
    RED = 231 << 16 | 72 << 8 | 86, // 0xE74856
    // TODO: Add more colors.
} Color;

void enableVirtualTerminalProcessing(void);
int getCursorPosition(void);
int getConsoleDimensions(void);
int getConsoleWidth(void);
int getConsoleHeight(void);

void setForegroundColor(int color);
void setBackgroundColor(int color);

static inline void clearConsole(void) {
    printf("\x1B[H\x1B[2J\x1B[3J");
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

static inline void resetCursorPosition(void) {
    printf("\x1B[H");
}
