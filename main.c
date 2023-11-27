#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

#include "console_utils.h"
#include "game.h"

typedef struct {
    const char* text;
    const uint16_t consoleRow;
} MenuOption;

uint8_t openMainMenu(uint8_t currentSelection);

void printCenteredText(const char* str) {
    printf("\x1B[%dG%s", (getConsoleWidth() - (int)strlen(str)) / 2, str);
}

inline void printTitle(void) {
    const char* asciiArt[] = {
        "        ___     _________________________________               ",
        "      //  //  //   ___    //\\\\_____    _____\\__  \\\\        ",
        "     //  //  //   /  /   //       \\\\   \\   \\   \\  \\\\     ",
        "    //  //  //   /  /   //         \\\\   \\   \\___\\  \\\\    ",
        "   //  //  //   /  /   //           \\\\   \\     _____\\\\     ",
        "  //  //  //   /  /   //             \\\\   \\     _    \\\\    ",
        " //  //__//___/__/   //               \\\\   \\    \\\\    \\\\ ",
        "//___________/______//                 \\\\___\\____\\\\____\\\\"
    };
    const uint32_t centerPoint = (getConsoleWidth() - 58) / 2;
    for (uint32_t i = 0; i < 8; i++) {
        for (uint32_t j = 0; j < centerPoint; j++) {
            putchar(' ');
        }
        puts(asciiArt[i]);
    }
    printf("\n");
    printCenteredText("THE BATTLE FOR MIDDLE-EARTH");
}

void openGameSetupSinglePlayer(void) {
    // TODO: Implement Single Player mode.
}

void openGameSetupMultiPlayer(void) {
    const uint16_t consoleWidth = getConsoleWidth();
    const uint16_t consoleHeight = getConsoleHeight();
    clearConsole();
    setForegroundColor(RED);
    for (uint16_t i = 1; i <= consoleHeight ; i++) {
        setCursorVerticalHorizontalPosition(i, 2);
        printf("┃");
    }
    setCursorVerticalHorizontalPosition(1, 4);
    printf("Player Setup");
    setCursorVerticalHorizontalPosition(2, 2);
    printf("┣");
    for (uint16_t i = 3; i <= consoleWidth ; i++) {
        printf("━");
    }
    showCursor();
    char player1Name[20], player2Name[20];
    setCursorVerticalHorizontalPosition(4, 4);
    printf("Player 1 Name: ");
    setForegroundColor(WHITE);
    readLine(player1Name, 20, false);
    setCursorVerticalHorizontalPosition(5, 4);
    setForegroundColor(RED);
    printf("Player 2 Name: ");
    setForegroundColor(WHITE);
    readLine(player2Name, 20, false);
    hideCursor();
    clearConsole();
    startNewMultiplayerGame(player1Name, player2Name, false);    
}

uint8_t openGameSetup(uint8_t currentSelection) {
    const uint32_t currentConsoleDimensions = getConsoleDimensions();
    const uint16_t consoleWidth = currentConsoleDimensions & 0xFFFF;
    const uint16_t consoleMidRow = (currentConsoleDimensions >> 16 & 0xFFFF) / 2;
    const MenuOption menuOptions[] = {
        {"Single Player (vs CPU)", consoleMidRow - 2},
        {"Multiplayer (2 Players)", consoleMidRow},
        {"Back", consoleMidRow + 3},
    };
    clearConsole();
    setForegroundColor(RED);
    printTitle();
    for (int i = 0; i < 3; i++) {
        printf("\x1B[%dH", menuOptions[i].consoleRow);
        if (currentSelection == i) {
            setForegroundColor(WHITE);
            printf("\x1B[%dG< %s >", (consoleWidth - ((int)strlen(menuOptions[currentSelection].text) + 4)) / 2, menuOptions[currentSelection].text);
        } else {
            setForegroundColor(RED);
            printCenteredText(menuOptions[i].text);
        }
    }
    int ch;
    do {
        // Reprint the entire menu if the console was resized.
        if (currentConsoleDimensions != getConsoleDimensions()) return openGameSetup(currentSelection);
        ch = _getch();
        const int previousSelection = currentSelection;
        if (ch == 0 || ch == 224) {
            switch (_getch()) {
                case 72: // Up Arrow
                    currentSelection = max(0, currentSelection - 1);
                    break;
                case 80: // Down Arrow
                    currentSelection = min(2, currentSelection + 1);
                    break;
            }
        }
        // Do nothing if the selection did not change.
        if (currentSelection == previousSelection) continue;
        clearLine(menuOptions[previousSelection].consoleRow);
        setForegroundColor(RED);
        printCenteredText(menuOptions[previousSelection].text);
        clearLine(menuOptions[currentSelection].consoleRow);
        setForegroundColor(WHITE);
        setCursorHorizontalPosition((uint16_t)(consoleWidth - (strlen(menuOptions[currentSelection].text) + 4)) / 2);
        printf("< %s >", menuOptions[currentSelection].text);
    } while (ch != 13 && ch != 32); // Pressing Enter (13) or Space (32) will exit the loop.
    switch (currentSelection) {
        case 0:
            openGameSetupSinglePlayer();
            break;
        case 1:
            openGameSetupMultiPlayer();
            break;
        case 2:
            openMainMenu(0);
            break;
        default:
            // It should be impossible to get here.
            return openGameSetup(currentSelection);
    }
    return currentSelection;
}

uint8_t openLoadGameMenu(uint8_t currentSelection) {
    // TODO: Implement Load Game Menu.
    return currentSelection;
}

uint8_t openSettingsMenu(uint8_t currentSelection) {
    // TODO: Implement Settings Menu.
    return currentSelection;
}

uint8_t openMainMenu(uint8_t currentSelection) {
    const uint32_t currentConsoleDimensions = getConsoleDimensions();
    const uint16_t consoleWidth = currentConsoleDimensions & 0xFFFF;
    const uint16_t consoleMidRow = (currentConsoleDimensions >> 16 & 0xFFFF) / 2;
    const MenuOption menuOptions[] = {
        {"Start Game", consoleMidRow - 3},
        {"Load  Game", consoleMidRow - 1},
        {"Settings", consoleMidRow + 1},
        {"Exit", consoleMidRow + 3},
    };
    resetTextDecoration();
    clearConsole();
    setForegroundColor(RED);
    printTitle();
    for (int i = 0; i < 4; i++) {
        printf("\x1B[%dH", menuOptions[i].consoleRow);
        if (currentSelection == i) {
            setForegroundColor(WHITE);
            printf("\x1B[%dG< %s >", (consoleWidth - ((int)strlen(menuOptions[currentSelection].text) + 4)) / 2, menuOptions[currentSelection].text);
        } else {
            setForegroundColor(RED);
            printCenteredText(menuOptions[i].text);
        }
    }
    int ch;
    do {
        // Reprint the entire menu if the console was resized.
        if (currentConsoleDimensions != getConsoleDimensions()) return openMainMenu(currentSelection);
        ch = _getch();
        const int previousSelection = currentSelection;
        if (ch == 0 || ch == 224) {
            switch (_getch()) {
                case 72: // Up Arrow
                    currentSelection = max(0, currentSelection - 1);
                    break;
                case 80: // Down Arrow
                    currentSelection = min(3, currentSelection + 1);
                    break;
            }
        }
        // Do nothing if the selection did not change.
        if (currentSelection == previousSelection) continue;
        clearLine(menuOptions[previousSelection].consoleRow);
        setForegroundColor(RED);
        printCenteredText(menuOptions[previousSelection].text);
        clearLine(menuOptions[currentSelection].consoleRow);
        setForegroundColor(WHITE);
        setCursorHorizontalPosition((uint16_t)(consoleWidth - (strlen(menuOptions[currentSelection].text) + 4)) / 2);
        printf("< %s >", menuOptions[currentSelection].text);
    } while (ch != 13 && ch != 32); // Pressing Enter (13) or Space (32) will exit the loop.
    switch (currentSelection) {
        case 0:
            openGameSetup(0);
            break;
        case 1:
            openLoadGameMenu(0);
            break;
        case 2:
            openSettingsMenu(0);
            break;
        case 3:
            exit(0);
        default:
            // It should be impossible to get here.
            return openMainMenu(currentSelection);
    }
    return currentSelection;
}

int main(void) {
    // TODO: Use atexit() to add a function that runs at the exit of the program to automatically save the game.
    
    setConsoleWindowTitle("The Battle for Middle-Earth");
    if (!setupConsole()) {
        hideCursor();
        // If we get here we must have failed to resize the console window. We'll ask the user to resize it.
        uint16_t consoleWidth = getConsoleWidth();
        uint16_t consoleHeight = getConsoleHeight();
        if (consoleWidth < 135 || consoleHeight < 45) {
            setCursorVerticalPosition(consoleHeight / 2 - 1);
            printCenteredText("Welcome to The Battle for Middle-Earth");
            printf("\n");
            printCenteredText("Please increase the window size for a better experience.");
            while (1) {
                consoleWidth = getConsoleWidth();
                consoleHeight = getConsoleHeight();
                if (consoleWidth >= 135 && consoleHeight >= 45) break;
                Sleep(1000);
            }
        }
    } else hideCursor();
    
    openMainMenu(0);
    
    printf("\n\nPress any key to continue...");
    (void)getchar();
    return 0;
}
