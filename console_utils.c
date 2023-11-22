#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "console_utils.h"

void enableVirtualTerminalProcessing(void) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (GetConsoleMode(hConsole, &dwMode)) SetConsoleMode(hConsole, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void setupConsole(void) {
    enableVirtualTerminalProcessing();
    // Set the console's input and output code page to UTF-8.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
}

void setForegroundColor(const int color) {
    const int r = color >> 16 & 0xFF;
    const int g = color >> 8 & 0xFF;
    const int b = color & 0xFF;
    printf("\x1B[38;2;%d;%d;%dm", r, g, b);
}

void setBackgroundColor(const int color) {
    const int r = color >> 16 & 0xFF;
    const int g = color >> 8 & 0xFF;
    const int b = color & 0xFF;
    printf("\x1B[48;2;%d;%d;%dm", r, g, b);
}

int getCursorPosition(void) {
    int cursorTop, cursorLeft;
    // Query the cursor position.
    printf("\x1B[6n");
    char cursorPosReport[100] = "";
    int i = 0, ch;
    while ((ch = _getch()) != 'R') {
        if (ch == EOF) break;
        if (isprint(ch) && i + 1 < 100) {
            cursorPosReport[i++] = (char)ch;
        }
    }
    cursorPosReport[i] = '\0';
    if (sscanf(cursorPosReport, "[%d;%d", &cursorTop, &cursorLeft) != 2) return 0;
    return cursorTop << 16 | cursorLeft;
}

int getConsoleDimensions(void) {
    // This is a little bit of an hack, but good enough.
    const int currentCursorPosition = getCursorPosition();
    // Move cursor the bottom right corner.
    printf("\x1B[9999;9999H");
    const int result = getCursorPosition();
    // Move cursor back to its original position.
    printf("\x1B[%d;%dH", currentCursorPosition >> 16 & 0xFFFF, currentCursorPosition & 0xFFFF);
    return result;
}

int getConsoleWidth(void) {
    return getConsoleDimensions() & 0xFFFF;
}

int getConsoleHeight(void) {
    return getConsoleDimensions() >> 16 & 0xFFFF;
}
