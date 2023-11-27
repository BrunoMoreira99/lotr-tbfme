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

uint32_t getConsoleDimensions(void) {
    // This is a little bit of an hack, but good enough.
    const uint32_t currentCursorPosition = getCursorPosition();
    // Move cursor the bottom right corner.
    printf("\x1B[9999;9999H");
    const uint32_t result = getCursorPosition();
    // Move cursor back to its original position.
    setCursorPosition(currentCursorPosition);
    return result;
}

uint16_t getConsoleWidth(void) {
    return getConsoleDimensions() & 0xFFFF;
}

uint16_t getConsoleHeight(void) {
    return getConsoleDimensions() >> 16 & 0xFFFF;
}

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
