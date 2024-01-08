#define _CRT_SECURE_NO_WARNINGS

#define _WIN32_WINNT 0x0500

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#include "console_utils.h"
#include "ui_io_utils.h"
#include "game.h"

typedef struct {
    uint8_t day;
    uint8_t month;
    uint16_t year;
} Date;

Date getDate(const uint64_t timestamp) {
    const time_t t = (time_t)timestamp;
    const struct tm *tm = localtime(&t);
    return (Date){(uint8_t)tm->tm_mday, (uint8_t)(tm->tm_mon + 1), (uint16_t)(tm->tm_year + 1900)};
}

void printFormattedTime(const uint64_t totalSeconds) {
    const uint64_t hours = totalSeconds / 3600;
    const uint64_t remainingSeconds = totalSeconds % 3600;
    const uint64_t minutes = remainingSeconds / 60;
    const uint64_t seconds = remainingSeconds % 60;

    if (hours > 0) {
        if (hours == 1) printf("1 hour, ");
        else printf("%llu hours, ", hours);
    }
    if (hours > 0 || minutes > 0) {
        if (minutes == 1) printf("1 minute, ");
        else printf("%llu minutes, ", minutes);
    }
    if (seconds == 1) printf("1 second");
    else printf("%llu seconds", seconds);
}

int16_t openMainMenu(int16_t currentSelection);

inline void printTitle(void) {
    const char *asciiArt[] = {
        "        ___     _________________________________               ",
        "      //  //  //   ___    //\\\\_____    _____\\__  \\\\        ",
        "     //  //  //   /  /   //       \\\\   \\   \\   \\  \\\\     ",
        "    //  //  //   /  /   //         \\\\   \\   \\___\\  \\\\    ",
        "   //  //  //   /  /   //           \\\\   \\     _____\\\\     ",
        "  //  //  //   /  /   //             \\\\   \\     _    \\\\    ",
        " //  //__//___/__/   //               \\\\   \\    \\\\    \\\\ ",
        "//___________/______//                 \\\\___\\____\\\\____\\\\"
    };
    const uint16_t centerPoint = (getConsoleWidth() - 58) / 2;
    for (uint16_t i = 0; i < 8; ++i) {
        for (uint16_t j = 0; j < centerPoint; ++j) {
            putchar(' ');
        }
        puts(asciiArt[i]);
    }
    printf("\n");
    printCenteredText("THE BATTLE FOR MIDDLE-EARTH");
}

uint32_t loadMapList(char*** mapList) {
    FILE *fp = fopen("Maps\\MapList.data", "r");
    if (fp == NULL) return 0;
    char *firstLine = fReadLine(fp);
    uint32_t mapCount = strtol(firstLine, NULL, 10);
    free(firstLine);
    *mapList = malloc(mapCount * sizeof(char*));
    for (uint32_t i = 0; i < mapCount; ++i) {
        char *mapName = fReadLine(fp);
        const int32_t bufSize = snprintf(NULL, 0, "Maps\\%s.map", mapName) + 1;
        char *mapFileName = malloc(bufSize);
        if (mapFileName != NULL) {
            const int32_t mFileNameLen = snprintf(mapFileName, bufSize, "Maps\\%s.map", mapName);
            if (!(mFileNameLen < 0 || mFileNameLen >= bufSize)) {
                FILE *mapFile = fopen(mapFileName, "r");
                free(mapFileName);
                if (mapFile != NULL) {
                    (void)fclose(mapFile);
                    (*mapList)[i] = mapName;
                    continue;
                }
            }
        }
        // If we got here, then the map file does not exist or an error occurred.
        free(mapName);
        --mapCount;
        --i;
    }
    (void)fclose(fp);
    if (!mapCount) free(*mapList);
    return mapCount;
}

void drawMapPreview(const char* mapName, const uint16_t row, const uint16_t column) {
    setCursorVerticalHorizontalPosition(row, column);
    const int32_t bufSize = snprintf(NULL, 0, "Maps\\%s.map", mapName) + 1;
    char *mapFileName = malloc(bufSize);
    if (mapFileName == NULL) return;
    const int32_t mFileNameLen = snprintf(mapFileName, bufSize, "Maps\\%s.map", mapName);
    if (mFileNameLen < 0 || mFileNameLen >= bufSize) return;
    FILE *fp = fopen(mapFileName, "r");
    if (fp == NULL) return;
    int ch;
    uint16_t r = 0;
    uint8_t player = 0;
    setForegroundColor(BLACK);
    while ((ch = fgetc(fp)) != EOF) {
        switch (ch) {
            case '\r': continue;
            case '\n': setCursorVerticalHorizontalPosition(++r + row, column); continue;
            case 'P' : setBackgroundColor(C_PLAIN);        break;
            case 'F' : setBackgroundColor(C_FOREST);       break;
            case 'M' : setBackgroundColor(C_MOUNTAIN);     break;
            case 'R' : setBackgroundColor(C_RIVER);        break;
            case 'W' : setBackgroundColor(C_WATER);        break;
            case 'B' : setBackgroundColor(C_BRIDGE);       break;
            case 'S' : setBackgroundColor(C_SNOW);         break;
            case 'L' : setBackgroundColor(C_LAVA);         break;
            case 'T' : setBackgroundColor(C_BASALT);       break;
            case '#' : printf("\b\bP%d", ++player); continue;
            default  : resetBackgroundColor();             break;
        }
        printf("  ");
    }
    (void)fclose(fp);
    resetBackgroundColor();
    resetForegroundColor();
}

char* openMapSelector(void) {
    char **mapList;
    const uint32_t mapCount = loadMapList(&mapList);
    if (!mapCount) return NULL;
    int currentSelection = 0;
    if (mapCount > 1) {
        const uint32_t currentConsoleDimensions = getConsoleDimensions();
        const uint16_t consoleWidth = currentConsoleDimensions & 0xFFFF;
        const uint16_t consoleHeight = currentConsoleDimensions >> 16 & 0xFFFF;
        clearConsole();
        setForegroundColor(RED);
        drawFullWidthBoxTitle("Map Selector");
        for (uint16_t i = 5; i <= consoleHeight; ++i) {
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
        for (uint32_t i = 1; i < mapCount; ++i) {
            setCursorVerticalHorizontalPosition(5 + (uint16_t)i * 2, 4);
            printf("  %s", mapList[i]);
        }
        int ch;
        do {
            if (currentConsoleDimensions != getConsoleDimensions()) {
                return openMapSelector();
            }
            const int previousSelection = currentSelection;
            ch = _getch();
            if (ch == 0 || ch == 224) ch = _getch();
            switch (ch) {
                case KEY_W: case KEY_UP:   --currentSelection; break;
                case KEY_S: case KEY_DOWN: ++currentSelection; break;
                case KEY_ENTER: case KEY_SPACE: break;
                default: continue;
            }
            currentSelection = (int32_t)((currentSelection + mapCount) % mapCount);
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
    const int32_t bufSize = snprintf(NULL, 0, "Maps\\%s.map", mapList[currentSelection]) + 1;
    char *chosenMapFile = malloc(bufSize);
    (void)snprintf(chosenMapFile, bufSize, "Maps\\%s.map", mapList[currentSelection]);
    for (uint32_t i = 0; i < mapCount; ++i) {
        free(mapList[i]);
    }
    free(mapList);
    return chosenMapFile;
}

void openGameSetupSinglePlayer(const uint8_t selectedSaveSlot) {
    const uint16_t consoleHeight = getConsoleHeight();
    clearConsole();
    setForegroundColor(RED);
    drawFullWidthBoxTitle("Player Setup");
    for (uint16_t i = 5; i <= consoleHeight; ++i) {
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
    char *chosenMapFile = openMapSelector();
    startNewSinglePlayerGame(selectedSaveSlot, chosenMapFile, playerName, false);    
}

void openGameSetupMultiPlayer(const uint8_t selectedSaveSlot) {
    const uint16_t consoleHeight = getConsoleHeight();
    clearConsole();
    setForegroundColor(RED);
    drawFullWidthBoxTitle("Player Setup");
    for (uint16_t i = 5; i <= consoleHeight ; ++i) {
        setCursorVerticalHorizontalPosition(i, 2);
        printf("┃");
    }
    showCursor();
    char player1Name[21], player2Name[21];
    setCursorVerticalHorizontalPosition(5, 4);
    printf("Player 1 Name: ");
    setForegroundColor(WHITE);
    readLine(player1Name, 21, false);
    setCursorVerticalHorizontalPosition(6, 4);
    setForegroundColor(RED);
    printf("Player 2 Name: ");
    setForegroundColor(WHITE);
    readLine(player2Name, 21, false);
    hideCursor();
    char *chosenMapFile = openMapSelector();
    startNewMultiplayerGame(selectedSaveSlot, chosenMapFile, player1Name, player2Name, false);    
}

int16_t makeMenu(const MenuOption* menuOptions, const int8_t numberOfOptions, int16_t currentSelection) {
    const uint32_t currentConsoleDimensions = getConsoleDimensions();
    const uint16_t consoleWidth = currentConsoleDimensions & 0xFFFF;
    const uint16_t consoleMidRow = (currentConsoleDimensions >> 16 & 0xFFFF) / 2;
    resetTextDecoration();
    clearConsole();
    setForegroundColor(RED);
    setCursorVerticalPosition(4);
    printTitle();
    for (int8_t i = 0; i < numberOfOptions; ++i) {
        setCursorVerticalPosition((uint16_t)(consoleMidRow + menuOptions[i].rowOffset));
        if (currentSelection == i) {
            setForegroundColor(menuOptions[i].disabled ? DARK_GRAY : WHITE);
            printf("\x1B[%dG< %s >", (consoleWidth - ((int)strlen(menuOptions[i].text) + 4)) / 2, menuOptions[i].text);
        } else {
            setForegroundColor(menuOptions[i].disabled ? DARK_GRAY : RED);
            printCenteredText(menuOptions[i].text);
        }
    }
    int ch;
    do {
        // Reprint the entire menu if the console was resized.
        if (currentConsoleDimensions != getConsoleDimensions()) {
            return makeMenu(menuOptions, numberOfOptions, currentSelection);
        }
        const int32_t previousSelection = currentSelection;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        switch (ch) {
            case KEY_W: case KEY_UP:   currentSelection = max(0, currentSelection - 1);                   break;
            case KEY_S: case KEY_DOWN: currentSelection = min(numberOfOptions - 1, currentSelection + 1); break;
            default: continue;
        }
        clearLine((uint16_t)(consoleMidRow + menuOptions[previousSelection].rowOffset));
        setForegroundColor(menuOptions[previousSelection].disabled ? DARK_GRAY : RED);
        printCenteredText(menuOptions[previousSelection].text);
        clearLine((uint16_t)(consoleMidRow + menuOptions[currentSelection].rowOffset));
        setForegroundColor(menuOptions[currentSelection].disabled ? DARK_GRAY : WHITE);
        setCursorHorizontalPosition((uint16_t)(consoleWidth - (strlen(menuOptions[currentSelection].text) + 4)) / 2);
        printf("< %s >", menuOptions[currentSelection].text);
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || menuOptions[currentSelection].disabled);
    return currentSelection;
}

void drawSaveSlotBlock_NewGame(const uint8_t slotId, const bool selected, const uint16_t row, const uint16_t column) {
    GameData *gameData = NULL;
    setForegroundColor(selected ? WHITE : RED);
    setCursorVerticalHorizontalPosition(row, column);
    drawBoldBox(50, 5);
    setCursorVerticalHorizontalPosition(row, column + 2);
    printf(" Save Slot %d ", slotId);
    if (loadGame(&gameData, slotId)) {
        setCursorVerticalHorizontalPosition(row + 2, column + 3);
        printf("%s vs. %s", gameData->players[0].name, gameData->players[1].name);
        const Date lastSaveDate = getDate(gameData->lastSaveTimestamp);
        setCursorVerticalHorizontalPosition(row, column + 36);
        printf(" %02d/%02d/%d ", lastSaveDate.day, lastSaveDate.month, lastSaveDate.year);
        setCursorVerticalHorizontalPosition(row + 3, column + 3);
        setForegroundColor(selected ? RED : DARK_GRAY);
        printf("Overwrite Save Game");
        free(gameData);
    } else {
        setCursorVerticalHorizontalPosition(row + 2, column + 3);
        printf("New Game");
    }
    setForegroundColor(RED);
}

bool drawSaveSlotBlock_LoadGame(const uint8_t slotId, const bool selected, const uint16_t row, const uint16_t column) {
    GameData *gameData = NULL;
    const bool loadedSuccessfully = loadGame(&gameData, slotId);
    setForegroundColor(loadedSuccessfully ? (selected ? WHITE : RED) : DARK_GRAY);
    setCursorVerticalHorizontalPosition(row, column);
    drawBoldBox(50, 5);
    setCursorVerticalHorizontalPosition(row, column + 2);
    printf(" Save Slot %d ", slotId);
    if (loadedSuccessfully) {
        setCursorVerticalHorizontalPosition(row + 2, column + 3);
        printf("%s vs. %s", gameData->players[0].name, gameData->players[1].name);
        const Date lastSaveDate = getDate(gameData->lastSaveTimestamp);
        setCursorVerticalHorizontalPosition(row, column + 36);
        printf(" %02d/%02d/%d ", lastSaveDate.day, lastSaveDate.month, lastSaveDate.year);
        setCursorVerticalHorizontalPosition(row + 3, column + 3);
        printf("Game Time: ");
        printFormattedTime(gameData->elapsedTimeSeconds);
        free(gameData);
    } else {
        setCursorVerticalHorizontalPosition(row + 2, column + 3);
        printf("Empty Slot");
    }
    setForegroundColor(RED);
    return loadedSuccessfully;
}

int32_t openGameSetupSaveSlotSelection(int8_t currentSelection) {
    const uint32_t currentConsoleDimensions = getConsoleDimensions();
    const uint16_t consoleWidth = currentConsoleDimensions & 0xFFFF;
    const uint16_t consoleHeight = currentConsoleDimensions >> 16 & 0xFFFF;
    const uint8_t blockHeight = 6, nSaveSlots = 6;
    const uint16_t startColumn = (consoleWidth - 50) / 2;
    const uint16_t startRow = (consoleHeight - (blockHeight * nSaveSlots - 1)) / 2 + (consoleHeight % 2 == 0 ? 2 : 1);
    clearConsole();
    setForegroundColor(RED);
    drawFullWidthBoxTitle("Select Save Slot");
    for (int8_t i = 0; i < nSaveSlots; ++i) {
        drawSaveSlotBlock_NewGame(i + 1, i == currentSelection, (uint16_t)(startRow + i * blockHeight), startColumn);
    }
    int ch;
    do {
        if (currentConsoleDimensions != getConsoleDimensions()) {
            return openGameSetupSaveSlotSelection(currentSelection);
        }
        const int8_t previousSelection = currentSelection;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        if (ch == KEY_ESC) return MENU_BACK;
        switch (ch) {
            case KEY_W: case KEY_UP:   currentSelection = max(0, currentSelection - 1); break;
            case KEY_S: case KEY_DOWN: currentSelection = min(5, currentSelection + 1); break;
            default: continue;
        }
        drawSaveSlotBlock_NewGame(previousSelection + 1, false, (uint16_t)(startRow + previousSelection * blockHeight), startColumn);
        drawSaveSlotBlock_NewGame(currentSelection + 1, true, (uint16_t)(startRow + currentSelection * blockHeight), startColumn);
    } while (ch != KEY_ENTER && ch != KEY_SPACE);
    return currentSelection;
}

void openGameSetup(void) {
    const int16_t selection = makeMenu((MenuOption[]){
        {"Single Player (vs CPU)", true, -2},
        {"Multiplayer (2 Players)", false, 0},
        {"Back", false, 3}
    }, 3, 1);
    if (selection == 2) openMainMenu(0);
    else {
        const int32_t saveSlotSelectionResult = openGameSetupSaveSlotSelection(0);
        if (saveSlotSelectionResult == MENU_BACK) {
            openGameSetup();
            return;
        }
        const uint8_t selectedSaveSlot = (uint8_t)(saveSlotSelectionResult + 1);
        switch (selection) {
            case 0: openGameSetupSinglePlayer(selectedSaveSlot); break;
            case 1: openGameSetupMultiPlayer(selectedSaveSlot);  break;
            default: exit(2); // It should be impossible to get here.
        }
    }
}

bool hasAnySavedGame(void) {
    for (uint8_t i = 1; i <= 6; ++i) {
        GameData *gameData = NULL;
        if (loadGame(&gameData, i)) {
            free(gameData);
            return true;
        }
    }
    return false;
}

void openLoadGameMenu(int8_t currentSelection) {
    const uint32_t currentConsoleDimensions = getConsoleDimensions();
    const uint16_t consoleWidth = currentConsoleDimensions & 0xFFFF;
    const uint16_t consoleHeight = currentConsoleDimensions >> 16 & 0xFFFF;
    const uint8_t blockHeight = 6, nSaveSlots = 6;
    const uint16_t startColumn = (consoleWidth - 50) / 2;
    const uint16_t startRow = (consoleHeight - (blockHeight * nSaveSlots - 1)) / 2 + (consoleHeight % 2 == 0 ? 2 : 1);
    clearConsole();
    setForegroundColor(RED);
    drawFullWidthBoxTitle("Load Game");
    bool saveSlotStates[6];
    for (int8_t i = 0; i < nSaveSlots; ++i) {
        saveSlotStates[i] = drawSaveSlotBlock_LoadGame(i + 1, i == currentSelection, (uint16_t)(startRow + i * blockHeight), startColumn);
    }
    int ch;
    do {
        if (currentConsoleDimensions != getConsoleDimensions()) {
            openLoadGameMenu(currentSelection);
            return;
        }
        const int8_t previousSelection = currentSelection;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        if (ch == KEY_ESC) return;
        switch (ch) {
            case KEY_W: case KEY_UP:
                do {
                    currentSelection = (int8_t)((currentSelection - 1 + nSaveSlots) % nSaveSlots);
                } while (!saveSlotStates[currentSelection]);
                break;
            case KEY_S: case KEY_DOWN:
                do {
                    currentSelection = (int8_t)((currentSelection + 1) % nSaveSlots);
                } while (!saveSlotStates[currentSelection]);
                break;
            default: continue;
        }
        if (currentSelection == previousSelection) continue;
        drawSaveSlotBlock_LoadGame(previousSelection + 1, false, (uint16_t)(startRow + previousSelection * blockHeight), startColumn);
        drawSaveSlotBlock_LoadGame(currentSelection + 1, true, (uint16_t)(startRow + currentSelection * blockHeight), startColumn);
    } while (ch != KEY_ENTER && ch != KEY_SPACE);
    resumeSaveGame(currentSelection + 1);
}

void openSettingsMenu(void) {
    // TODO: Implement Settings Menu.
}

int16_t openMainMenu(const int16_t currentSelection) {
    const int16_t selection = makeMenu((MenuOption[]){
        {"Start Game", false, -3},
        {"Load  Game", !hasAnySavedGame(), -1},
        {"Settings", true, 1},
        {"Exit", false, 3}
    }, 4, currentSelection);
    switch (selection) {
        case 0: openGameSetup(); break;
        case 1: openLoadGameMenu(0); break;
        case 2: openSettingsMenu(); break;
        case 3: exit(0);
        default: exit(2); // It should be impossible to get here.
    }
    return selection;
}

int main(void) {
    if (!setupConsole("The Battle for Middle-Earth")) {
        hideCursor();
        // If we get here we must have failed to resize the console window. We'll ask the user to resize it.
        enforceConsoleResize(
            "Welcome to The Battle for Middle-Earth",
            "Please increase the window size and/or reduce the font size to continue.",
            135,
            46
        );
    } else hideCursor();
    int16_t currentSelection = 0;
    while (true) {
        currentSelection = openMainMenu(currentSelection);
    }
}
