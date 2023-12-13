#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include <windows.h>
#include "console_utils.h"

void enableVirtualTerminalProcessing(const HANDLE hConsoleOut) {
    // Enable Virtual Terminal Processing to ensure that the console accepts ANSI Escape Codes.
    // See https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences for more info.
    DWORD dwMode = 0;
    if (GetConsoleMode(hConsoleOut, &dwMode)) SetConsoleMode(hConsoleOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

bool maximizeConsole(const HANDLE hConsoleOut) {
    const HWND hConsoleWindow = GetConsoleWindow();
    if (!hConsoleWindow) return false;
    if (!ShowWindow(hConsoleWindow, SW_MAXIMIZE)) return false;
    COORD largestSize = GetLargestConsoleWindowSize(hConsoleOut);
    if (!largestSize.X && !largestSize.Y) return false;
    --largestSize.X;
    --largestSize.Y;
    return SetConsoleScreenBufferSize(hConsoleOut, largestSize);
}

bool setupConsole(void) {
    const HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsoleOut != INVALID_HANDLE_VALUE) {
        enableVirtualTerminalProcessing(hConsoleOut);
    }
    // Set the console's input and output code page to UTF-8.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    return maximizeConsole(hConsoleOut);
}

void setConsoleWindowTitle(const char* str) {
    printf("\x1B]0;%s\x07", str);
}

void setForegroundColor(const uint32_t color) {
    const uint8_t r = color >> 16 & 0xFF;
    const uint8_t g = color >> 8 & 0xFF;
    const uint8_t b = color & 0xFF;
    printf("\x1B[38;2;%d;%d;%dm", r, g, b);
}

void setBackgroundColor(const uint32_t color) {
    const uint8_t r = color >> 16 & 0xFF;
    const uint8_t g = color >> 8 & 0xFF;
    const uint8_t b = color & 0xFF;
    printf("\x1B[48;2;%d;%d;%dm", r, g, b);
}

uint32_t getCursorPosition(void) {
    /* Implementation using ANSI Escape Codes and _getch to read the keyboard input buffer and thus preventing the
     * echoing of the cursor's position into the console.
     * The usage of ANSI Escape Codes is nice as it is relatively "multi-platform", but we are using _getch anyway
     * (which is a MS-DOS/Win library) so we might as well just use GetConsoleScreenBufferInfo from the WinAPI.
     * Since we are targeting Windows anyway, the WinAPI is preferable here as _getch was falling into a race condition.
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
    */
    const HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    if (!GetConsoleScreenBufferInfo(hConsoleOut, &consoleInfo)) return 1 << 16 | 1;
    return (consoleInfo.dwCursorPosition.Y + 1) << 16 | (consoleInfo.dwCursorPosition.X + 1);
}

uint32_t getConsoleDimensions(void) {
    /* Implementation using ANSI Escape Codes.
    // This is a little bit of an hack, but good enough.
    const uint32_t currentCursorPosition = getCursorPosition();
    // Move cursor the bottom right corner.
    printf("\x1B[9999;9999H");
    const uint32_t result = getCursorPosition();
    // Move cursor back to its original position.
    setCursorPosition(currentCursorPosition);
    return result;
    */
    const HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    if (!GetConsoleScreenBufferInfo(hConsoleOut, &consoleInfo)) return 30 << 16 | 120;
    return consoleInfo.dwSize.Y << 16 | consoleInfo.dwSize.X;
}

uint16_t getConsoleWidth(void) {
    return getConsoleDimensions() & 0xFFFF;
}

uint16_t getConsoleHeight(void) {
    return getConsoleDimensions() >> 16 & 0xFFFF;
}
