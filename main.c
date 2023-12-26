#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

#include "console_utils.h"
#include "ui_io_utils.h"
#include "game.h"

int8_t openMainMenu(int8_t currentSelection);

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

int loadMapList(char*** mapList) {
    FILE *fp = fopen("Maps\\MapList.data", "r");
    if (fp == NULL) return 0;
    char *firstLine = fReadLine(fp);
    int mapCount = strtol(firstLine, NULL, 10);
    free(firstLine);
    *mapList = malloc(mapCount * sizeof(char*));
    for (int i = 0; i < mapCount; i++) {
        char *mapName = fReadLine(fp);
        char* mapFileName = malloc(10 + strlen(mapName));
        (void)sprintf(mapFileName, "Maps\\%s.map", mapName);
        FILE *mapFile = fopen(mapFileName, "r");
        free(mapFileName);
        if (mapFile == NULL) {
            // If we got here, then the map file does not exist.
            free(mapName);
            --mapCount;
            --i;
            continue;
        }
        (void)fclose(mapFile);
        (*mapList)[i] = mapName;
    }
    (void)fclose(fp);
    return mapCount;
}

void drawMapPreview(const char* mapName, const uint16_t row, const uint16_t column) {
    setCursorVerticalHorizontalPosition(row, column);
    char* mapFileName = malloc(10 + strlen(mapName));
    (void)sprintf(mapFileName, "Maps\\%s.map", mapName);
    FILE *fp = fopen(mapFileName, "r");
    if (fp == NULL) return;
    int ch;
    uint16_t r = 0;
    uint8_t player = 0;
    setForegroundColor(BLACK);
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            setCursorVerticalHorizontalPosition(++r + row, column);
            continue;
        }
        switch (ch) {
            case 'P':
                setBackgroundColor(C_PLAIN);
                break;
            case 'F':
                setBackgroundColor(C_FOREST);
                break;
            case 'M':
                setBackgroundColor(C_MOUNTAIN);
                break;
            case 'R':
                setBackgroundColor(C_RIVER);
                break;
            case 'W':
                setBackgroundColor(C_WATER);
                break;
            case 'B':
                setBackgroundColor(C_BRIDGE);
                break;
            case 'S':
                setBackgroundColor(C_SNOW);
                break;
            case 'L':
                setBackgroundColor(C_LAVA);
                break;
            case 'T':
                setBackgroundColor(C_BASALT);
                break;
            case '#':
                printf("\b\bP%d", ++player);
                continue;
            default:
                resetBackgroundColor();
                break;
        }
        printf("  ");
    }
    (void)fclose(fp);
    resetBackgroundColor();
    resetForegroundColor();
}

char* openMapSelector(void) {
    char **mapList;
    const int mapCount = loadMapList(&mapList);
    if (!mapCount) return NULL;
    int currentSelection = 0;
    if (mapCount > 1) {
        const uint16_t consoleWidth = getConsoleWidth();
        const uint16_t consoleHeight = getConsoleHeight();
        clearConsole();
        setForegroundColor(RED);
        drawFullWidthBoxTitle("Map Selector");
        for (uint16_t i = 5; i <= consoleHeight ; i++) {
            setCursorVerticalHorizontalPosition(i, 2);
            printf("┃");
        }
        const uint16_t mapPreviewPosRow = (consoleHeight - 19) / 2 + 1;
        const uint16_t mapPreviewPosColumn = consoleWidth - consoleWidth / 4 - 26;
        setCursorVerticalHorizontalPosition(mapPreviewPosRow - 1, mapPreviewPosColumn - 1);
        drawBoldBox(54, 19);
        setForegroundColor(WHITE);
        drawMapPreview(mapList[0], mapPreviewPosRow, mapPreviewPosColumn);
        setCursorVerticalHorizontalPosition(5, 4);
        printf("> %s", mapList[0]);
        setForegroundColor(RED);
        for (int i = 1; i < mapCount; i++) {
            setCursorVerticalHorizontalPosition(5 + (uint16_t)i * 2, 4);
            printf("  %s", mapList[i]);
        }
        int ch;
        do {
            const int previousSelection = currentSelection;
            ch = _getch();
            if (ch == 0 || ch == 224) ch = _getch();
            switch (ch) {
                case KEY_W:
                case KEY_UP:
                    --currentSelection;
                    break;
                case KEY_S:
                case KEY_DOWN:
                    ++currentSelection;
                    break;
            }
            currentSelection = (currentSelection + mapCount) % mapCount;
            if (currentSelection == previousSelection) continue;
            setCursorVerticalHorizontalPosition(5 + (uint16_t)previousSelection * 2, 4);
            setForegroundColor(RED);
            printf("  %s", mapList[previousSelection]);
            setCursorVerticalHorizontalPosition(5 + (uint16_t)currentSelection * 2, 4);
            setForegroundColor(WHITE);
            printf("> %s", mapList[currentSelection]);
            drawMapPreview(mapList[currentSelection], mapPreviewPosRow, mapPreviewPosColumn);
            setForegroundColor(RED);
        } while (ch != KEY_ENTER && ch != KEY_SPACE);
    }
    char* chosenMapFile = malloc(10 + strlen(mapList[currentSelection]));
    (void)sprintf(chosenMapFile, "Maps\\%s.map", mapList[currentSelection]);
    for (int i = 0; i < mapCount; i++) {
        free(mapList[i]);
    }
    free(mapList);
    return chosenMapFile;
}

void openGameSetupSinglePlayer(void) {
    const uint16_t consoleHeight = getConsoleHeight();
    clearConsole();
    setForegroundColor(RED);
    drawFullWidthBoxTitle("Player Setup");
    for (uint16_t i = 5; i <= consoleHeight ; i++) {
        setCursorVerticalHorizontalPosition(i, 2);
        printf("┃");
    }
    showCursor();
    char playerName[20];
    setCursorVerticalHorizontalPosition(5, 4);
    printf("Enter your name: ");
    setForegroundColor(WHITE);
    readLine(playerName, 20, false);
    hideCursor();
    char* chosenMapFile = openMapSelector();
    clearConsole();
    startNewSinglePlayerGame(chosenMapFile, playerName, false);    
}

void openGameSetupMultiPlayer(void) {
    const uint16_t consoleHeight = getConsoleHeight();
    clearConsole();
    setForegroundColor(RED);
    drawFullWidthBoxTitle("Player Setup");
    for (uint16_t i = 5; i <= consoleHeight ; i++) {
        setCursorVerticalHorizontalPosition(i, 2);
        printf("┃");
    }
    showCursor();
    char player1Name[20], player2Name[20];
    setCursorVerticalHorizontalPosition(5, 4);
    printf("Player 1 Name: ");
    setForegroundColor(WHITE);
    readLine(player1Name, 20, false);
    setCursorVerticalHorizontalPosition(6, 4);
    setForegroundColor(RED);
    printf("Player 2 Name: ");
    setForegroundColor(WHITE);
    readLine(player2Name, 20, false);
    hideCursor();
    char* chosenMapFile = openMapSelector();
    clearConsole();
    startNewMultiplayerGame(chosenMapFile, player1Name, player2Name, false);    
}

int8_t makeMenu(const MenuOption* menuOptions, const int8_t numberOfOptions, int8_t currentSelection) {
    const uint32_t currentConsoleDimensions = getConsoleDimensions();
    const uint16_t consoleWidth = currentConsoleDimensions & 0xFFFF;
    const uint16_t consoleMidRow = (currentConsoleDimensions >> 16 & 0xFFFF) / 2;
    resetTextDecoration();
    clearConsole();
    setForegroundColor(RED);
    printTitle();
    for (int i = 0; i < numberOfOptions; i++) {
        setCursorVerticalPosition((uint16_t)(consoleMidRow + menuOptions[i].rowOffset));
        if (currentSelection == i) {
            setForegroundColor(WHITE);
            printf("\x1B[%dG< %s >", (consoleWidth - ((int)strlen(menuOptions[i].text) + 4)) / 2, menuOptions[i].text);
        } else {
            setForegroundColor(RED);
            printCenteredText(menuOptions[i].text);
        }
    }
    int ch;
    do {
        // Reprint the entire menu if the console was resized.
        if (currentConsoleDimensions != getConsoleDimensions()) return makeMenu(menuOptions, numberOfOptions, currentSelection);
        const int8_t previousSelection = currentSelection;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        switch (ch) {
            case KEY_W:
            case KEY_UP:
                currentSelection = max(0, currentSelection - 1);
                break;
            case KEY_S:
            case KEY_DOWN:
                currentSelection = min(numberOfOptions - 1, currentSelection + 1);
                break;
            default:
                // Do nothing if the selection did not change.
                continue;
        }
        clearLine((uint16_t)(consoleMidRow + menuOptions[previousSelection].rowOffset));
        setForegroundColor(RED);
        printCenteredText(menuOptions[previousSelection].text);
        clearLine((uint16_t)(consoleMidRow + menuOptions[currentSelection].rowOffset));
        setForegroundColor(WHITE);
        setCursorHorizontalPosition((uint16_t)(consoleWidth - (strlen(menuOptions[currentSelection].text) + 4)) / 2);
        printf("< %s >", menuOptions[currentSelection].text);
    } while (ch != KEY_ENTER && ch != KEY_SPACE);
    return currentSelection;
}

void openGameSetup(void) {
    const int8_t selection = makeMenu((MenuOption[]){
        {"Single Player (vs CPU)", -2},
        {"Multiplayer (2 Players)", 0},
        {"Back", 3}
    }, 3, 0);
    switch (selection) {
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
            // It should be impossible to get here. Exit with code error 2.
            exit(2);
    }
}

void openLoadGameMenu(void) {
    // TODO: Implement Load Game Menu.
}

void openSettingsMenu(void) {
    // TODO: Implement Settings Menu.
}

void openMainMenu(void) {
    const int8_t selection = makeMenu((MenuOption[]){
        {"Start Game", -3},
        {"Load  Game", -1},
        {"Settings", 1},
        {"Exit", 3}
    }, 4, 0);
    switch (selection) {
        case 0:
            openGameSetup();
            break;
        case 1:
            openLoadGameMenu();
            break;
        case 2:
            openSettingsMenu();
            break;
        case 3:
            exit(0);
        default:
            // It should be impossible to get here. Exit with code error 2.
            exit(2);
    }
}

int main(void) {
    // TODO: Use atexit() to add a function that runs at the exit of the program to automatically save the game.

    if (!setupConsole("The Battle for Middle-Earth")) {
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
    while (true) {
        openMainMenu(0);
    }
}
