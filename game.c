#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <assert.h>

#include "console_utils.h"
#include "math_utils.h"
#include "ui_io_utils.h"
#include "game.h"

const GameSettings GameSettings_Default = {
    5,
    100, 100, 100, 100, 100,
    100, 100, 100,
    30, 20, 25, 25, 30,
    10, 15, 20,
    2, 1, 3,
    5, 7, 10
};

GameSettings *GameSettings_Current;

GameSettings* getGameSettings(void) {
    if (GameSettings_Current == NULL) {
        if ((GameSettings_Current = malloc(sizeof(GameSettings))) == NULL) {
            return (GameSettings*)&GameSettings_Default;
        }
        memcpy(GameSettings_Current, &GameSettings_Default, sizeof(GameSettings));
    }
    return GameSettings_Current;
}

Color getTerrainBackgroundColor(const TerrainType terrainType) {
    switch (terrainType) {
        case PLAIN:
            return C_PLAIN;
        case FOREST:
            return C_FOREST;
        case MOUNTAIN:
            return C_MOUNTAIN;
        case RIVER:
            return C_RIVER;
        case WATER:
            return C_WATER;
        case BRIDGE:
            return C_BRIDGE;
        case SNOW:
            return C_SNOW;
        case LAVA:
            return C_LAVA;
        case BASALT:
            return C_BASALT;
    }
    return BLACK;
}

bool canBuildInCell(const GameData* gameData, const GameBoardCell* cell) {
    if (cell->entityType != EMPTY_CELL) return false;
    switch (cell->terrainType) {
        case BRIDGE:
        case RIVER:
        case WATER:
        case LAVA:
            return false;
        default:
            return true;
    }
}

uint16_t getNumberOfMines(const GameData* gameData, const uint8_t player) {
    uint16_t minesCount = 0;
    for (int i = 0; i < 17; i++) {
        for (int j = 0; j < 26; j++) {
            const GameBoardCell cell = gameData->board[i][j];
            if (cell.owner == player && cell.entityType == MINES) {
                ++minesCount;
            }
        }
    }
    return minesCount;
}

void advanceTurn(GameData* gameData) {
    // Change to next player.
    gameData->currentPlayerTurn = ++gameData->currentPlayerTurn % 2;
    if (gameData->currentPlayerTurn == 0) ++gameData->currentRound;
    // Add Castar Coins based on the player's number of mines.
    const uint16_t playerMinesCount = getNumberOfMines(gameData, gameData->currentPlayerTurn);
    if (playerMinesCount) currentPlayer(gameData).coins += playerMinesCount * getGameSettings()->MINE_INCOME;
}

bool buildEntity(GameBoardCell* cell, Player* player, const EntityType entityType) {
    // If the cell is not empty, something went wrong.
    assert(cell->entityType == EMPTY_CELL);
    const GameSettings *gameSettings = getGameSettings();
    uint16_t eCost = 0;
    uint16_t eHealth = 0;
    switch (entityType) {
        case EMPTY_CELL:
            return false;
        case BASE:
            eCost = gameSettings->BASE_COST;
            eHealth = gameSettings->BASE_HEALTH;
            break;
        case MINES:
            eCost = gameSettings->MINES_COST;
            eHealth = gameSettings->MINES_HEALTH;
            break;
        case BARRACKS:
            eCost = gameSettings->BARRACKS_COST;
            eHealth = gameSettings->BARRACKS_HEALTH;
            break;
        case STABLES:
            eCost = gameSettings->STABLES_COST;
            eHealth = gameSettings->STABLES_HEALTH;
            break;
        case ARMOURY:
            eCost = gameSettings->ARMOURY_COST;
            eHealth = gameSettings->ARMOURY_HEALTH;
            break;
        case INFANTRY:
            eCost = gameSettings->INFANTRY_SPAWN_COST;
            eHealth = gameSettings->INFANTRY_HEALTH;
            break;
        case CAVALRY:
            eCost = gameSettings->CAVALRY_SPAWN_COST;
            eHealth = gameSettings->CAVALRY_HEALTH;
            break;
        case ARTILLERY:
            eCost = gameSettings->ARTILLERY_SPAWN_COST;
            eHealth = gameSettings->ARTILLERY_HEALTH;
            break;
    }
    if (player->coins < eCost) return false;
    cell->owner = player->id;
    cell->entityType = entityType;
    cell->health = (int16_t)eHealth;
    player->coins -= eCost;
    return true;
}

char* getCellStrRepr(const GameData* gameData, const GameBoardCell* cell) {
    switch (cell->entityType) {
        case BASE:
            return gameData->players[cell->owner].isMordor ? "MMM" : "GGG";
        case MINES:
            return " M ";
        case BARRACKS:
            return " B ";
        case STABLES:
            return " S ";
        case ARMOURY:
            return " F ";
        case INFANTRY:
            return " I ";
        case CAVALRY:
            return " C ";
        case ARTILLERY:
            return " A ";
        case EMPTY_CELL:
            return "   ";
    }
    return "   ";
}

void printGameCell(const GameData* gameData, const GameBoardCell* cell) {
    // If the cell is not empty but it has no owner, something went wrong.
    assert(!(cell->entityType != EMPTY_CELL && cell->owner == NO_OWNER));
    setBackgroundColor(getTerrainBackgroundColor(cell->terrainType));
    if (cell->owner != NO_OWNER) {
        setForegroundColor(gameData->players[cell->owner].isMordor ? C_MORDOR : C_GONDOR);
    }
    printf("%s", getCellStrRepr(gameData, cell));
    resetForegroundColor();
}

void printGameBoard(const GameData* gameData) {
    resetTextDecoration();
    printf("   ");
    for (int i = 'A'; i <= 'Z'; i++) {
        printf(" %c  ", i);
    }
    printf("\n  ╭");
    for (int i = 0; i < 25; i++) {
        printf("────");
    }
    printf("───╮\n");
    for (int i = 0; i < 16; i++) {
        if (i < 10) printf(" %d│", i);
        else printf("%d│", i);
        for (int j = 0; j < 25; j++) {
            printGameCell(gameData, &gameData->board[i][j]);
            if (gameData->board[i][j].terrainType != gameData->board[i][j + 1].terrainType) {
                resetBackgroundColor();
            }
            setForegroundColor(BLACK);
            printf("│");
            resetForegroundColor();
        }
        printGameCell(gameData, &gameData->board[i][25]);
        resetBackgroundColor();
        printf("│\n  │");
        setForegroundColor(BLACK);
        for (int j = 0; j < 25; j++) {
            if (gameData->board[i][j].terrainType == gameData->board[i + 1][j].terrainType) {
                setBackgroundColor(getTerrainBackgroundColor(gameData->board[i][j].terrainType));
                printf("───");
                if (gameData->board[i][j].terrainType != gameData->board[i][j + 1].terrainType ||
                    gameData->board[i][j].terrainType != gameData->board[i + 1][j + 1].terrainType
                ) {
                    resetBackgroundColor();
                }
                printf("┼");
            } else {
                printf("───┼");
            }
        }
        if (gameData->board[i][25].terrainType == gameData->board[i + 1][25].terrainType) {
            setBackgroundColor(getTerrainBackgroundColor(gameData->board[i][25].terrainType));
        }
        printf("───");
        resetBackgroundColor();
        resetForegroundColor();
        printf("│\n");
    }
    printf("16│");  
    for (int j = 0; j < 25; j++) {
        printGameCell(gameData, &gameData->board[16][j]);
        if (gameData->board[16][j].terrainType != gameData->board[16][j + 1].terrainType) {
            resetBackgroundColor();
        }
        setForegroundColor(BLACK);
        printf("│");
        resetForegroundColor();
    }
    printGameCell(gameData, &gameData->board[16][25]);
    resetBackgroundColor();
    printf("│\n  ╰");
    for (int i = 0; i < 25; i++) {
        printf("────");
    }
    printf("───╯\n");
}

void printUnitList(GameData* gameData) {
    setCursorVerticalHorizontalPosition(2, 109);
    drawBoxWithTitleAndFooter("Units", NULL, 26, 35, drawRoundedBox);
}

void clearTurnInfoBox(void) {
    for (uint16_t i = 0; i < 7; i++) {
        setCursorVerticalHorizontalPosition(38 + i, 109);
        clearCurrentLineFromCursorForward();
    }
}

void clearActionsMenu(void) {
    for (uint16_t i = 0; i < 7; i++) {
        setCursorVerticalHorizontalPosition(38 + i, 108);
        clearCurrentLineFromCursorBackward();
    }
}

GameBoardCell* getSelectedGameBoardCell(GameData* gameData, Int16Vector2* currentCoord, bool (*isValidCell)(const GameData*, const GameBoardCell*)) {
    for (int16_t i = 0; i < 16; i++) {
        for (int16_t j = 0; j < 27; j++) {
            if (gameData->board[i][j].entityType == BASE && gameData->board[i][j].owner == gameData->currentPlayerTurn) {
                currentCoord->x = i;
                currentCoord->y = j;
                i = 16;
                break;
            }
        }
    }
    bool isSelectedCellValid;
    int ch;
    do {
        const GameBoardCell *currentCell = &gameData->board[currentCoord->x][currentCoord->y];
        setCursorVerticalHorizontalPosition(1 + 2 * (currentCoord->x + 1), 4 * (currentCoord->y + 1));
        enableBlinking();
        if ((isSelectedCellValid = isValidCell(gameData, currentCell))) {
            printf("███");
        } else {
            setBackgroundColor(getTerrainBackgroundColor(currentCell->terrainType));
            setForegroundColor(RED);
            printf("╳╳╳");
            resetBackgroundColor();
            resetForegroundColor();
        }
        disableBlinking();
        const Int16Vector2 previousCoord = *currentCoord;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        switch (ch) {
            case KEY_W:
            case KEY_UP:
                --currentCoord->x;
                break;
            case KEY_S:
            case KEY_DOWN:
                ++currentCoord->x;
                break;
            case KEY_D:
            case KEY_RIGHT:
                ++currentCoord->y;
                break;
            case KEY_A:
            case KEY_LEFT:
                --currentCoord->y;
                break;
        }
        setCursorVerticalHorizontalPosition(1 + 2 * (previousCoord.x + 1), 4 * (previousCoord.y + 1));
        printGameCell(gameData, &gameData->board[previousCoord.x][previousCoord.y]);
        resetBackgroundColor();
        if (ch == KEY_ESC) return NULL;
        currentCoord->x = (int16_t)((currentCoord->x + 17) % 17);
        currentCoord->y = (int16_t)((currentCoord->y + 26) % 26);
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || !isSelectedCellValid);
    return &gameData->board[currentCoord->x][currentCoord->y];
}

int32_t makeActionMenu(const GameData* gameData, const char* title, const char* footer, ActionMenuOption menuOptions[], const uint8_t numberOfOptions, int32_t* selection) {
    clearActionsMenu();
    setCursorVerticalHorizontalPosition(37, 3);
    drawBoxWithTitleAndFooter(title, footer, 105, 9, drawRoundedBox);
    int32_t currentSelection = selection == NULL ? 0 : *selection;
    for (uint8_t i = 0; i < numberOfOptions; i++) {
        setCursorVerticalHorizontalPosition(38 + menuOptions[i].row * 2, menuOptions[i].consoleColumn);
        if (currentSelection == i) {
            printf("\x1B[5m>\x1B[25m %s", menuOptions[i].text);
        } else {
            printf("  %s", menuOptions[i].text);
        }
        if (menuOptions[i].castarCoinCost > 0) {
            setForegroundColor(menuOptions[i].castarCoinCost > currentPlayer(gameData).coins ? RED : GREEN);
            printf(" (₵%d)", menuOptions[i].castarCoinCost);
            resetForegroundColor();
        }
    }
    bool canAffordSelection = true;
    int ch;
    do {
        ch = _getch();
        if (ch == KEY_ESC) return MENU_BACK;
        const int32_t previousSelection = currentSelection;
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
            case KEY_D:
            case KEY_RIGHT:
                currentSelection += 2;
                break;
            case KEY_A:
            case KEY_LEFT:
                currentSelection -= 2;
                break;
            default:
                continue;
        }
        currentSelection = (currentSelection + numberOfOptions) % numberOfOptions;
        if (selection != NULL) *selection = currentSelection;
        if (menuOptions[currentSelection].castarCoinCost > 0) {
            canAffordSelection = menuOptions[currentSelection].castarCoinCost <= currentPlayer(gameData).coins;
        }
        setCursorVerticalHorizontalPosition(38 + menuOptions[previousSelection].row * 2, menuOptions[previousSelection].consoleColumn);
        printf("  %s", menuOptions[previousSelection].text);
        setCursorVerticalHorizontalPosition(38 + menuOptions[currentSelection].row * 2, menuOptions[currentSelection].consoleColumn);
        printf("\x1B[5m>\x1B[25m %s", menuOptions[currentSelection].text);
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || !canAffordSelection);
    return currentSelection;
}

int32_t openExitGameMenu(void) {
    return makeActionMenu(NULL, "Exit Game", MENU_FOOTER_GO_BACK, (ActionMenuOption[]){
        {"Save and Exit to Desktop", 0, 1, 10},
        {"Save and Exit to Main Menu", 0, 2, 10},
        {"Exit to Desktop without saving", 0, 1, 60},
        {"Exit to Main Menu without saving", 0, 2, 60}
    }, 4, NULL);
}

int32_t openCreateBuildingMenu(GameData* gameData) {
    const GameSettings *gameSettings = getGameSettings();
    ActionMenuOption actionMenu[] = {
        {"Mines", gameSettings->MINES_COST, 1, 10},
        {"Barracks", gameSettings->BARRACKS_COST, 2, 10},
        {"Stables", gameSettings->STABLES_COST, 1, 42},
        {"Armoury", gameSettings->ARMOURY_COST, 2, 42}
    };
    const int32_t selection = makeActionMenu(gameData, "Build...", MENU_FOOTER_GO_BACK, actionMenu, 4, NULL);
    if (selection == MENU_BACK) return selection;
    const EntityType selectedBuilding = selection + 2;
    clearActionsMenu();
    setCursorVerticalHorizontalPosition(37, 3);
    char *boxTitle = malloc((strlen(actionMenu[selection].text) + 10) * sizeof(char));
    (void)sprintf(boxTitle, "Build %s...", actionMenu[selection].text);
    drawBoxWithTitleAndFooter(boxTitle, MENU_FOOTER_CANCEL, 105, 9, drawRoundedBox);
    free(boxTitle);
    setCursorVerticalHorizontalPosition(40, 12);
    printf("Navigate the map with the arrow keys or WASD to choose where you want to build.");
    setCursorVerticalHorizontalPosition(42, 12);
    printf("Press SPACE or ENTER to confirm the location.");
    Int16Vector2 selectedCellCoord;
    GameBoardCell* selectedCell = getSelectedGameBoardCell(gameData, &selectedCellCoord, canBuildInCell);
    if (selectedCell == NULL) return openCreateBuildingMenu(gameData);
    buildEntity(selectedCell, &currentPlayer(gameData), selectedBuilding);
    setCursorVerticalHorizontalPosition(1 + 2 * (selectedCellCoord.x + 1), 4 * (selectedCellCoord.y + 1));
    printGameCell(gameData, selectedCell);
    resetBackgroundColor();
    return selection;
}

int32_t openMainActionsMenu(const GameData* gameData, int32_t* currentSelection) {
    return makeActionMenu(gameData, "Actions", NULL, (ActionMenuOption[]){
        {"Build", 0, 1, 10},
        {"Spawn Unit", 0, 2, 10},
        {"Select Building", 0, 1, 42},
        {"Select Unit", 0, 2, 42},
        {"End Turn", 0, 1, 80},
        {"Exit Game", 0, 2, 80}
    }, 6, currentSelection);
}

void printTurnInfoBox(const GameData* gameData) {
    clearTurnInfoBox();
    setCursorVerticalHorizontalPosition(37, 109);
    char *roundInfo = malloc((getNumDigits((int32_t)gameData->currentRound + 1) + 7) * sizeof(char));
    (void)sprintf(roundInfo, "Round %d", gameData->currentRound + 1);
    drawBoxWithTitleAndFooter(roundInfo, NULL, 26, 9, drawRoundedBox);
    free(roundInfo);
    setCursorVerticalHorizontalPosition(39, 118);
    printf("Player %d", gameData->currentPlayerTurn + 1);
    const char *currentPlayerName = currentPlayer(gameData).name;
    setCursorVerticalHorizontalPosition(40, 109 + (uint16_t)(26 - strlen(currentPlayerName)) / 2);
    printf("%s", currentPlayerName);
    setCursorVerticalHorizontalPosition(42, 116);
    const int32_t playerCoins = currentPlayer(gameData).coins;
    printf("Castar Coins");
    setCursorVerticalHorizontalPosition(43, 122 - (max(3, (uint16_t)getNumDigits(playerCoins)) + 2) / 2);
    printf("₵%3d", playerCoins);
}

void Game(GameData* gameData) {
    printGameBoard(gameData);
    while (!gameData->isGameOver) {
        printUnitList(gameData);
        printTurnInfoBox(gameData);
        int32_t currentSelection = 0;
        while (true) {
            const int32_t action = openMainActionsMenu(gameData, &currentSelection);
            if (action == 0) {
                openCreateBuildingMenu(gameData);
                break;
            } else if (action == 1) {
                // TODO: Implement Spawn Unit
            } else if (action == 2) {
                // TODO: Implement Select Building
            } else if (action == 3) {
                // TODO: Implement Select Unit
            } else if (action == 4) {
                advanceTurn(gameData);
                break;
            } else {
                const int32_t exitResult = openExitGameMenu();
                switch (exitResult) {
                    case 0:
                    case 1:
                        // saveGame(gameData);
                        break;
                }
                switch (exitResult) {
                    case 0:
                    case 2:
                        exit(0);
                    case 1:
                    case 3:
                        // Return to Main Menu.
                        return;
                }
            }
        }
        if (currentPlayer(gameData).coins <= 0) advanceTurn(gameData);
    }
}

void createBoardFromMapFile(char* mapFileName, GameData* gameData) {
    FILE *fp = fopen(mapFileName, "r");
    if (fp == NULL) return;
    uint8_t player = 0;
    for (int i = 0; i < 17; i++) {
        for (int j = 0; j < 26; j++) {
            const int ch = fgetc(fp);
            if (ch == EOF) {
                i = 17;
                break;
            }
            switch (ch) {
                default:
                case 'P':
                    gameData->board[i][j].terrainType = PLAIN;
                    break;
                case 'F':
                    gameData->board[i][j].terrainType = FOREST;
                    break;
                case 'M':
                    gameData->board[i][j].terrainType = MOUNTAIN;
                    break;
                case 'R':
                    gameData->board[i][j].terrainType = RIVER;
                    break;
                case 'W':
                    gameData->board[i][j].terrainType = WATER;
                    break;
                case 'B':
                    gameData->board[i][j].terrainType = BRIDGE;
                    break;
                case 'S':
                    gameData->board[i][j].terrainType = SNOW;
                    break;
                case 'L':
                    gameData->board[i][j].terrainType = LAVA;
                    break;
                case 'T':
                    gameData->board[i][j].terrainType = BASALT;
                    break;
                case '#':
                    buildEntity(&gameData->board[i][--j], &gameData->players[player++], BASE);
                    break;
            }
        }
        // Skip the line feed (\n) at the end of each row
        (void)fgetc(fp);
    }
    (void)fclose(fp);
    free(mapFileName);
}

void startNewSinglePlayerGame(char* mapFile, const char* playerName, const bool player1IsMordor) {
    GameData gameData = {
        time(NULL),
        0,
        {
            {0, "Player 1", 100, player1IsMordor, false },
            {1, "CPU", 100, !player1IsMordor, true }
        },
        0,
        false,
        {{{PLAIN, EMPTY_CELL, NO_OWNER, 0}}}
    };
    createBoardFromMapFile(mapFile, &gameData);
    strcpy(gameData.players[0].name, playerName);
    Game(&gameData);
}

void startNewMultiplayerGame(char* mapFile, const char* player1Name, const char* player2Name, const bool player1IsMordor) {
    GameData gameData = {
        time(NULL),
        0,
        {
            {0, "Player 1", 100, player1IsMordor, false },
            {1, "Player 2", 100, !player1IsMordor, false }
        },
        0,
        false,
        {{{PLAIN, EMPTY_CELL, NO_OWNER, 0}}}
    };
    createBoardFromMapFile(mapFile, &gameData);
    strcpy(gameData.players[0].name, player1Name);
    strcpy(gameData.players[1].name, player2Name);
    Game(&gameData);
}
