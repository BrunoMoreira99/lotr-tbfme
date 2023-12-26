#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <assert.h>

#include "console_utils.h"
#include "ui_io_utils.h"
#include "game.h"

const GameSettings GameSettings_Default = {
    5,
    100, 50, 70, 70, 70,
    30, 40, 20,
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

void clearEntityListBox(void) {
    for (uint16_t i = 0; i < 33; i++) {
        setCursorVerticalHorizontalPosition(3 + i, 109);
        clearFromCursorForward(24);
    }
}

void clearTurnInfoBox(void) {
    for (uint16_t i = 0; i < 7; i++) {
        setCursorVerticalHorizontalPosition(38 + i, 109);
        clearFromCursorForward(24);
    }
}

void clearActionsMenu(void) {
    for (uint16_t i = 0; i < 7; i++) {
        setCursorVerticalHorizontalPosition(38 + i, 4);
        clearFromCursorForward(103);
    }
}

void drawActionsMenu(const char* title, const char* footer, const bool clear) {
    if (clear) clearActionsMenu();
    setCursorVerticalHorizontalPosition(37, 3);
    drawBoxWithTitleAndFooter(title, footer, 105, 9, drawRoundedBox);
}

Int16Vector2 getPlayerBaseCoordinate(const GameData* gameData, const uint8_t playerId) {
    for (int16_t y = 0; y < 17; y++) {
        for (int16_t x = 0; x < 26; x++) {
            if (gameData->board[y][x].entityType == BASE && gameData->board[y][x].owner == playerId) {
                return (Int16Vector2){x, y};
            }
        }
    }
    return (Int16Vector2){0, 0};
}

uint32_t getTerrainBackgroundColor(const TerrainType terrain) {
    switch (terrain) {
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

bool isTerrainPassable(const TerrainType terrain) {
    switch (terrain) {
        case RIVER:
        case LAVA:
            return false;
    }
    return true;
}

bool canUnitStepOnCell(const GameBoardCell* cell) {
    if (cell->entityType != EMPTY_CELL) return false;
    if (!isTerrainPassable(cell->terrainType)) return false;
    return true;
}

bool canBuildAtCoordinate(const GameDataExtended* gameDataEx, const Int16Vector2 cellCoord) {
    if (getManhattanDistance(gameDataEx->currentPlayerBaseCoord, cellCoord) > 4) {
        return false;
    }
    const GameBoardCell* cell = &gameDataEx->gameData.board[cellCoord.y][cellCoord.x];
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

List getValidCellsToBuild(const GameDataExtended* gameDataEx) {
    const GameData *gameData = &gameDataEx->gameData;
    List validCells = List_init();
    for (int16_t y = 0; y < 17; y++) {
        for (int16_t x = 0; x < 26; x++) {
            if (gameData->board[y][x].entityType != EMPTY_CELL) continue;
            if (canBuildAtCoordinate(gameDataEx, (Int16Vector2){x, y})) {
                Int16Vector2 *c = malloc(sizeof(Int16Vector2));
                c->x = x;
                c->y = y;
                List_append(&validCells, c);
            }
        }
    }
    return validCells;
}

bool hasValidCellsToBuild(const GameDataExtended* gameDataEx) {
    const GameData *gameData = &gameDataEx->gameData;
    for (int16_t y = 0; y < 17; y++) {
        for (int16_t x = 0; x < 26; x++) {
            if (gameData->board[y][x].entityType != EMPTY_CELL) continue;
            if (canBuildAtCoordinate(gameDataEx, (Int16Vector2){x, y})) {
                return true;
            }
        }
    }
    return false;
}

List getValidCellsToSpawnUnit(const GameData* gameData, const EntityType unit) {
    List validCells = List_init();
    EntityType requiredBuilding;
    switch (unit) {
        case INFANTRY:
            requiredBuilding = BARRACKS;
            break;
        case CAVALRY:
            requiredBuilding = STABLES;
            break;
        case ARTILLERY:
            requiredBuilding = ARMOURY;
            break;
        default:
            return validCells;
    }
    for (int16_t y = 0; y < 17; y++) {
        for (int16_t x = 0; x < 26; x++) {
            const GameBoardCell *cell = &gameData->board[y][x];
            if (cell->owner != gameData->currentPlayerTurn || cell->entityType != requiredBuilding) continue;
            // Add all valid adjacent cells to the validCells list.
            if (x > 0 && canUnitStepOnCell(&gameData->board[y][x - 1])) {
                Int16Vector2 *cln = malloc(sizeof(Int16Vector2));
                cln->x = (int16_t)x - 1;
                cln->y = y;
                List_append(&validCells, cln);
            }
            if (x < 25 && canUnitStepOnCell(&gameData->board[y][x + 1])) {
                Int16Vector2 *crn = malloc(sizeof(Int16Vector2));
                crn->x = (int16_t)x + 1;
                crn->y = y;
                List_append(&validCells, crn);
            }
            if (y > 0 && canUnitStepOnCell(&gameData->board[y - 1][x])) {
                Int16Vector2 *ctn = malloc(sizeof(Int16Vector2));
                ctn->x = x;
                ctn->y = (int16_t)y - 1;
                List_append(&validCells, ctn);
            }
            if (y < 16 && canUnitStepOnCell(&gameData->board[y + 1][x])) {
                Int16Vector2 *cbn = malloc(sizeof(Int16Vector2));
                cbn->x = x;
                cbn->y = (int16_t)y + 1;
                List_append(&validCells, cbn);
            }
            if (x > 0 && y > 0 &&  canUnitStepOnCell(&gameData->board[y - 1][x - 1])) {
                Int16Vector2 *ctln = malloc(sizeof(Int16Vector2));
                ctln->x = (int16_t)x - 1;
                ctln->y = (int16_t)y - 1;
                List_append(&validCells, ctln);
            }
            if (x > 0 && y < 16 && canUnitStepOnCell(&gameData->board[y + 1][x - 1])) {
                Int16Vector2 *ctrn = malloc(sizeof(Int16Vector2));
                ctrn->x = (int16_t)x - 1;
                ctrn->y = (int16_t)y + 1;
                List_append(&validCells, ctrn);
            }
            if (x < 25 && y > 0 && canUnitStepOnCell(&gameData->board[y - 1][x + 1])) {
                Int16Vector2 *cbln = malloc(sizeof(Int16Vector2));
                cbln->x = (int16_t)x + 1;
                cbln->y = (int16_t)y - 1;
                List_append(&validCells, cbln);
            }
            if (x < 25 && y < 16 && canUnitStepOnCell(&gameData->board[y + 1][x + 1])) {
                Int16Vector2 *cbrn = malloc(sizeof(Int16Vector2));
                cbrn->x = (int16_t)x + 1;
                cbrn->y = (int16_t)y + 1;
                List_append(&validCells, cbrn);
            }
        }
    }
    return validCells;
}

uint16_t getNumberOfMines(const GameData* gameData, const uint8_t player) {
    uint16_t minesCount = 0;
    for (int y = 0; y < 17; y++) {
        for (int x = 0; x < 26; x++) {
            const GameBoardCell cell = gameData->board[y][x];
            if (cell.owner == player && cell.entityType == MINES) {
                ++minesCount;
            }
        }
    }
    return minesCount;
}

char* getEntityName(const EntityType entityType, const bool ownerIsMordor) {
    switch (entityType) {
        case BASE:
            return "Base";
        case MINES:
            return "Mines";
        case BARRACKS:
            return "Barracks";
        case STABLES:
            return "Stables";
        case ARMOURY:
            return "Armoury";
        case INFANTRY:
            return ownerIsMordor ? "Orc Warriors" : "Citadel Guards";
        case CAVALRY:
            return ownerIsMordor ? "Wargs" : "Swan-Knights";
        case ARTILLERY:
            return ownerIsMordor ? "Siege Towers" : "Trebuchets";
    }
    return NULL;
}

EntityInfo getEntityInfo(const EntityType entityType) {
    const GameSettings* gameSettings = getGameSettings();
    switch (entityType) {
        case BASE:
            return (EntityInfo){
                entityType,
                gameSettings->BASE_HEALTH,
                gameSettings->BASE_COST,
                0,
                0
            };
        case MINES:
            return (EntityInfo){
                entityType,
                gameSettings->MINES_HEALTH,
                gameSettings->MINES_COST,
                0,
                0
            };
        case BARRACKS:
            return (EntityInfo){
                entityType,
                gameSettings->BARRACKS_HEALTH,
                gameSettings->BARRACKS_COST,
                0,
                0
            };
        case STABLES:
            return (EntityInfo){
                entityType,
                gameSettings->STABLES_HEALTH,
                gameSettings->STABLES_COST,
                0,
                0
            };
        case ARMOURY:
            return (EntityInfo){
                entityType,
                gameSettings->ARMOURY_HEALTH,
                gameSettings->ARMOURY_COST,
                0,
                0
            };
        case INFANTRY:
            return (EntityInfo){
                entityType,
                gameSettings->INFANTRY_HEALTH,
                gameSettings->INFANTRY_SPAWN_COST,
                gameSettings->INFANTRY_MOVEMENT_COST,
                gameSettings->INFANTRY_ATTACK_POWER
            };
        case CAVALRY:
            return (EntityInfo){
                entityType,
                gameSettings->CAVALRY_HEALTH,
                gameSettings->CAVALRY_SPAWN_COST,
                gameSettings->CAVALRY_MOVEMENT_COST,
                gameSettings->CAVALRY_ATTACK_POWER
            };
        case ARTILLERY:
            return (EntityInfo){
                entityType,
                gameSettings->ARTILLERY_HEALTH,
                gameSettings->ARTILLERY_SPAWN_COST,
                gameSettings->ARTILLERY_MOVEMENT_COST,
                gameSettings->ARTILLERY_ATTACK_POWER
            };
    }
    return (EntityInfo){ entityType, 0, 0, 0, 0 };
}

void refreshCurrentPlayerEntityLists(GameDataExtended* gameDataEx) {
    List_clear(&gameDataEx->currentPlayerBuildingsList);
    List_clear(&gameDataEx->currentPlayerUnitsList);
    Int16Vector2 *baseCoord = malloc(sizeof(Int16Vector2));
    *baseCoord = gameDataEx->currentPlayerBaseCoord;
    List_append(&gameDataEx->currentPlayerBuildingsList, baseCoord);
    for (int16_t y = 0; y < 17; y++) {
        for (int16_t x = 0; x < 26; x++) {
            const GameBoardCell *cell = &gameDataEx->gameData.board[y][x];
            if (cell->entityType <= BASE || cell->owner != gameDataEx->gameData.currentPlayerTurn) continue;
            Int16Vector2 *cellCoord = malloc(sizeof(Int16Vector2));
            *cellCoord = (Int16Vector2){x, y};
            if (cell->entityType >= MINES && cell->entityType <= ARMOURY) {
                List_append(&gameDataEx->currentPlayerBuildingsList, cellCoord);
            } else {
                List_append(&gameDataEx->currentPlayerUnitsList, cellCoord);
            }
        }
    }
}

bool createEntity(GameBoardCell* cell, Player* player, const EntityType entityType) {
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

void advanceTurn(GameDataExtended* gameDataEx) {
    // Change to next player.
    gameDataEx->gameData.currentPlayerTurn = ++gameDataEx->gameData.currentPlayerTurn % 2;
    gameDataEx->currentPlayer = &gameDataEx->gameData.players[gameDataEx->gameData.currentPlayerTurn];
    gameDataEx->currentPlayerBaseCoord = getPlayerBaseCoordinate(&gameDataEx->gameData, gameDataEx->gameData.currentPlayerTurn);
    refreshCurrentPlayerEntityLists(gameDataEx);
    if (gameDataEx->gameData.currentPlayerTurn == 0) ++gameDataEx->gameData.currentRound;
    // Add Castar Coins based on the player's number of mines.
    const uint16_t playerMinesCount = getNumberOfMines(&gameDataEx->gameData, gameDataEx->gameData.currentPlayerTurn);
    if (playerMinesCount) gameDataEx->currentPlayer->coins += playerMinesCount * getGameSettings()->MINE_INCOME;
}

void setCursorAtCellCoord(const Int16Vector2 cellCoord) {
    setCursorVerticalHorizontalPosition(1 + 2 * (cellCoord.y + 1), 4 * (cellCoord.x + 1));
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

void printGameCell(const GameData* gameData, const GameBoardCell* cell, const bool darken) {
    // If the cell is not empty but it has no owner, something went wrong.
    assert(!(cell->entityType != EMPTY_CELL && cell->owner == NO_OWNER));
    const uint32_t terrainColor = getTerrainBackgroundColor(cell->terrainType);
    setBackgroundColor(darken ? darkenColor(terrainColor, 0.30f) : terrainColor);
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
    for (int y = 0; y < 16; y++) {
        if (y < 10) printf(" %d│", y);
        else printf("%d│", y);
        for (int x = 0; x < 25; x++) {
            printGameCell(gameData, &gameData->board[y][x], false);
            if (gameData->board[y][x].terrainType != gameData->board[y][x + 1].terrainType) {
                resetBackgroundColor();
            }
            setForegroundColor(BLACK);
            printf("│");
            resetForegroundColor();
        }
        printGameCell(gameData, &gameData->board[y][25], false);
        resetBackgroundColor();
        printf("│\n  │");
        setForegroundColor(BLACK);
        for (int x = 0; x < 25; x++) {
            if (gameData->board[y][x].terrainType == gameData->board[y + 1][x].terrainType) {
                setBackgroundColor(getTerrainBackgroundColor(gameData->board[y][x].terrainType));
                printf("───");
                if (gameData->board[y][x].terrainType != gameData->board[y][x + 1].terrainType ||
                    gameData->board[y][x].terrainType != gameData->board[y + 1][x + 1].terrainType
                ) {
                    resetBackgroundColor();
                }
                printf("┼");
            } else {
                printf("───┼");
            }
        }
        if (gameData->board[y][25].terrainType == gameData->board[y + 1][25].terrainType) {
            setBackgroundColor(getTerrainBackgroundColor(gameData->board[y][25].terrainType));
        }
        printf("───");
        resetBackgroundColor();
        resetForegroundColor();
        printf("│\n");
    }
    printf("16│");  
    for (int x = 0; x < 25; x++) {
        printGameCell(gameData, &gameData->board[16][x], false);
        if (gameData->board[16][x].terrainType != gameData->board[16][x + 1].terrainType) {
            resetBackgroundColor();
        }
        setForegroundColor(BLACK);
        printf("│");
        resetForegroundColor();
    }
    printGameCell(gameData, &gameData->board[16][25], false);
    resetBackgroundColor();
    printf("│\n  ╰");
    for (int i = 0; i < 25; i++) {
        printf("────");
    }
    printf("───╯\n");
}

void printTurnInfoBox(const GameDataExtended* gameDataEx) {
    clearTurnInfoBox();
    setCursorVerticalHorizontalPosition(37, 109);
    const size_t bufSize = snprintf(NULL, 0, "Round %d", gameDataEx->gameData.currentRound + 1) + 1;
    char *roundInfo = malloc(bufSize);
    (void)snprintf(roundInfo, bufSize, "Round %d", gameDataEx->gameData.currentRound + 1);
    drawBoxWithTitleAndFooter(roundInfo, NULL, 26, 9, drawRoundedBox);
    free(roundInfo);
    setCursorVerticalHorizontalPosition(39, 118);
    printf("Player %d", gameDataEx->gameData.currentPlayerTurn + 1);
    const char *currentPlayerName = gameDataEx->currentPlayer->name;
    setCursorVerticalHorizontalPosition(40, 109 + (uint16_t)(26 - strlen(currentPlayerName)) / 2);
    printf("%s", currentPlayerName);
    setCursorVerticalHorizontalPosition(42, 116);
    const int32_t playerCoins = gameDataEx->currentPlayer->coins;
    printf("Castar Coins");
    setCursorVerticalHorizontalPosition(43, 122 - (max(3, (uint16_t)getNumDigits(playerCoins)) + 2) / 2);
    printf("₵%3d", playerCoins);
}

void printEntityList(const GameDataExtended* gameDataEx, const uint8_t mode) {
    clearEntityListBox();
    setCursorVerticalHorizontalPosition(2, 109);
    drawBoxWithTitleAndFooter(mode == 0 ? "Buildings and Units" : mode == 1 ? "Buildings" : "Units", NULL, 26, 35, drawRoundedBox);
    uint16_t count = 0;
    Node *node;
    if (mode == 2) {
        node = gameDataEx->currentPlayerUnitsList.head;
    } else {
        node = gameDataEx->currentPlayerBuildingsList.head;
    }
    while (node) {
        const Int16Vector2 *cellCoord = node->data;
        const GameBoardCell *cell = &gameDataEx->gameData.board[cellCoord->y][cellCoord->x];
        if (count < 16) { // The box can only fit a maximum of 16 elements.
            setCursorVerticalHorizontalPosition(4 + count * 2, 111);
            printf("%s", getEntityName(cell->entityType, gameDataEx->gameData.players[cell->owner].isMordor));
            setCursorVerticalHorizontalPosition(4 + count * 2, cellCoord->y > 9 ? 129 : 130);
            printf("%c-%d", 'A' + cellCoord->x, cellCoord->y);
            setCursorVerticalHorizontalPosition(5 + count * 2, 111);
            const uint16_t maxHealth = getEntityInfo(cell->entityType).maxHealth;
            const double remainingHPFactor = (double)cell->health / maxHealth;
            const uint16_t remainingHealthBarWidth = (uint16_t)(14 * remainingHPFactor);
            const uint16_t lostHealthBarWidth = 14 - remainingHealthBarWidth;
            uint32_t healthColor;
            if (remainingHPFactor <= 1.0/3) {
                healthColor = RED;
            } else if (remainingHPFactor <= 2.0/3) {
                healthColor = YELLOW;
            } else {
                healthColor = GREEN;
            }
            setForegroundColor(healthColor);
            for (uint16_t i = 0; i < remainingHealthBarWidth; ++i) {
                printf("━");
            }
            if (lostHealthBarWidth) {
                setForegroundColor(DARK_GRAY);
                for (uint16_t i = 0; i < lostHealthBarWidth; ++i) {
                    printf("━");
                }
            }
            setForegroundColor(healthColor);
            printf(" %*s%d/%d", 6 - (getNumDigits(cell->health) + getNumDigits(maxHealth)), "", cell->health, maxHealth);
            resetForegroundColor();
        }
        if (mode == 0 && node == gameDataEx->currentPlayerBuildingsList.tail) {
            node = gameDataEx->currentPlayerUnitsList.head;
        } else {
            node = node->next;
        }
        ++count;
    }
    if (count > 16) {
        setCursorVerticalHorizontalPosition(36, 126);
        printf(" · · · ");
    }
}

bool isValidCell(const Int16Vector2 cellCoord, const List validCells) {
    Node *e = validCells.head;
    while (e) {
        const Int16Vector2 *eData = e->data;
        if (eData->x == cellCoord.x && eData->y == cellCoord.y) return true;
        e = e->next;
    }
    return false;
}

void drawValidCells(const GameData* gameData, const List validCells, const bool darken) {
    Node *e = validCells.head;
    while (e) {
        const Int16Vector2 *eData = e->data;
        setCursorAtCellCoord(*eData);
        printGameCell(gameData, &gameData->board[eData->y][eData->x], darken);
        e = e->next;
    }
    resetBackgroundColor();
}

GameBoardCell* getSelectedGameBoardCell(GameDataExtended* gameDataEx, Int16Vector2* currentCoord, const List validCells) {
    drawValidCells(&gameDataEx->gameData, validCells, true);
    bool isSelectedCellValid;
    int ch;
    do {
        const GameBoardCell *currentCell = &gameDataEx->gameData.board[currentCoord->y][currentCoord->x];
        setCursorAtCellCoord(*currentCoord);
        enableBlinking();
        if ((isSelectedCellValid = isValidCell(*currentCoord, validCells))) {
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
                --currentCoord->y;
                break;
            case KEY_S:
            case KEY_DOWN:
                ++currentCoord->y;
                break;
            case KEY_D:
            case KEY_RIGHT:
                ++currentCoord->x;
                break;
            case KEY_A:
            case KEY_LEFT:
                --currentCoord->x;
                break;
        }
        setCursorAtCellCoord(previousCoord);
        printGameCell(&gameDataEx->gameData, &gameDataEx->gameData.board[previousCoord.y][previousCoord.x], isValidCell(previousCoord, validCells));
        resetBackgroundColor();
        if (ch == KEY_ESC) {
            drawValidCells(&gameDataEx->gameData, validCells, false);
            return NULL;
        }
        currentCoord->x = (int16_t)((currentCoord->x + 26) % 26);
        currentCoord->y = (int16_t)((currentCoord->y + 17) % 17);
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || !isSelectedCellValid);
    drawValidCells(&gameDataEx->gameData, validCells, false);
    return &gameDataEx->gameData.board[currentCoord->y][currentCoord->x];
}

int32_t makeActionMenu(
    const GameDataExtended* gameDataEx,
    ActionMenuOption menuOptions[],
    const uint8_t numberOfOptions,
    int32_t* selection
) {
    int32_t currentSelection = selection == NULL ? 0 : *selection;
    for (uint8_t i = 0; i < numberOfOptions; i++) {
        setCursorVerticalHorizontalPosition(38 + menuOptions[i].row * 2, menuOptions[i].consoleColumn);
        if (menuOptions[i].disabled) {
            setForegroundColor(DARK_GRAY);
            printf("  %s", menuOptions[i].text);
            resetForegroundColor();
        } else {
            printf("  %s", menuOptions[i].text);
        }
        if (menuOptions[i].castarCoinCost > 0) {
            setForegroundColor(menuOptions[i].castarCoinCost > gameDataEx->currentPlayer->coins ? RED : GREEN);
            printf(" (₵%d)", menuOptions[i].castarCoinCost);
            resetForegroundColor();
        }
    }
    bool canAffordSelection = true;
    int ch;
    do {
        if (menuOptions[currentSelection].castarCoinCost > 0) {
            canAffordSelection = menuOptions[currentSelection].castarCoinCost <= gameDataEx->currentPlayer->coins;
        }
        setCursorVerticalHorizontalPosition(38 + menuOptions[currentSelection].row * 2, menuOptions[currentSelection].consoleColumn);
        if (menuOptions[currentSelection].disabled) {
            setForegroundColor(DARK_GRAY);
            printf("> %s", menuOptions[currentSelection].text);
            resetForegroundColor();
        } else {
            printf("\x1B[5m>\x1B[25m %s", menuOptions[currentSelection].text);
        }
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
        setCursorVerticalHorizontalPosition(38 + menuOptions[previousSelection].row * 2, menuOptions[previousSelection].consoleColumn);
        if (menuOptions[previousSelection].disabled) setForegroundColor(DARK_GRAY);
        printf("  %s", menuOptions[previousSelection].text);
        if (menuOptions[previousSelection].disabled) resetForegroundColor();
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || !canAffordSelection || menuOptions[currentSelection].disabled);
    return currentSelection;
}

int32_t openExitGameMenu(void) {
    drawActionsMenu("Exit Game", MENU_FOOTER_GO_BACK, true);
    return makeActionMenu(NULL, (ActionMenuOption[]){
        {"Save and Exit to Desktop", 0, false, 1, 10},
        {"Save and Exit to Main Menu", 0, false, 2, 10},
        {"Exit to Desktop without saving", 0, false, 1, 60},
        {"Exit to Main Menu without saving", 0, false, 2, 60}
    }, 4, NULL);
}

int32_t openCreateBuildingMenu(GameDataExtended* gameDataEx, int32_t currentSelection) {
    drawActionsMenu("Build...", MENU_FOOTER_GO_BACK, true);
    if (!hasValidCellsToBuild(gameDataEx)) {
        setCursorVerticalHorizontalPosition(41, 12);
        setForegroundColor(RED);
        printf("No land space available to build on.");
        resetForegroundColor();
        while (_getch() != KEY_ESC) {}
        return MENU_BACK;
    }
    const GameSettings *gameSettings = getGameSettings();
    ActionMenuOption actionMenu[] = {
        {"Mines", gameSettings->MINES_COST, false, 1, 10},
        {"Barracks", gameSettings->BARRACKS_COST, false, 2, 10},
        {"Stables", gameSettings->STABLES_COST, false, 1, 42},
        {"Armoury", gameSettings->ARMOURY_COST, false, 2, 42}
    };
    const int32_t selection = makeActionMenu(gameDataEx, actionMenu, 4, &currentSelection);
    if (selection == MENU_BACK) return selection;
    List validCells = getValidCellsToBuild(gameDataEx);
    const EntityType selectedBuilding = selection + 2;
    const size_t bufSize = snprintf(NULL, 0, "Build %s...", actionMenu[selection].text) + 1;
    char *boxTitle = malloc(bufSize);
    (void)snprintf(boxTitle, bufSize, "Build %s...", actionMenu[selection].text);
    drawActionsMenu(boxTitle, MENU_FOOTER_CANCEL, true);
    free(boxTitle);
    setCursorVerticalHorizontalPosition(40, 12);
    printf("Navigate the map with the arrow keys or WASD to choose where you want to build.");
    setCursorVerticalHorizontalPosition(42, 12);
    printf("Press [SPACE] or [ENTER] to confirm the location.");
    Int16Vector2 selectedCellCoord = gameDataEx->currentPlayerBaseCoord;
    GameBoardCell* selectedCell = getSelectedGameBoardCell(gameDataEx, &selectedCellCoord, validCells);
    List_clear(&validCells);
    if (selectedCell == NULL) return openCreateBuildingMenu(gameDataEx, selection); // If it's NULL, the user must have pressed ESC.
    createEntity(selectedCell, gameDataEx->currentPlayer, selectedBuilding);
    setCursorAtCellCoord(selectedCellCoord);
    printGameCell(&gameDataEx->gameData, selectedCell, false);
    resetBackgroundColor();
    return selection;
}

int32_t openSpawnUnitMenu(GameDataExtended* gameDataEx, int32_t currentSelection) {
    const bool playerIsMordor = gameDataEx->currentPlayer->isMordor;
    const GameSettings *gameSettings = getGameSettings();
    const ActionMenuOption menuOptions[] = {
        {playerIsMordor ? "Orc Warriors (Infantry)" : "Citadel Guards (Infantry)", gameSettings->INFANTRY_SPAWN_COST, false, 1, 10},
        {playerIsMordor ? "Wargs (Cavalry)" : "Swan-Knights (Cavalry)", gameSettings->CAVALRY_SPAWN_COST, false, 2, 10},
        {playerIsMordor ? "Siege Towers (Artillery)" : "Trebuchets (Artillery)", gameSettings->ARTILLERY_SPAWN_COST, false, 1, 60}
    };
    const char *buildingRequirements[] = {"Barracks", "Stables", "Armoury"};
    List unitValidCells[] = {
        getValidCellsToSpawnUnit(&gameDataEx->gameData, INFANTRY),
        getValidCellsToSpawnUnit(&gameDataEx->gameData, CAVALRY),
        getValidCellsToSpawnUnit(&gameDataEx->gameData, ARTILLERY)
    };
    drawActionsMenu("Spawn Unit...", MENU_FOOTER_GO_BACK, true);
    for (uint8_t i = 0; i < 3; i++) {
        setCursorVerticalHorizontalPosition(38 + menuOptions[i].row * 2, menuOptions[i].consoleColumn);
        printf("  %s", menuOptions[i].text);
        if (menuOptions[i].castarCoinCost > 0) {
            setForegroundColor(menuOptions[i].castarCoinCost > gameDataEx->currentPlayer->coins ? RED : GREEN);
            printf(" (₵%d)", menuOptions[i].castarCoinCost);
            resetForegroundColor();
        }
    }
    bool canAffordSelection = true, unitHasValidCells;
    int ch;
    do {
        unitHasValidCells = unitValidCells[currentSelection].length;
        if (!unitHasValidCells) {
            setForegroundColor(RED);
            setCursorVerticalHorizontalPosition(38 + 2 * 2, 60);
            printf("  Requires a free space next to %-8s", buildingRequirements[currentSelection]);
            resetForegroundColor();
        } else {
            setCursorVerticalHorizontalPosition(38 + 2 * 2, 60);
            clearFromCursorForward(40);
        }
        if (menuOptions[currentSelection].castarCoinCost > 0) {
            canAffordSelection = menuOptions[currentSelection].castarCoinCost <= gameDataEx->currentPlayer->coins;
        }
        setCursorVerticalHorizontalPosition(38 + menuOptions[currentSelection].row * 2, menuOptions[currentSelection].consoleColumn);
        printf("\x1B[5m>\x1B[25m %s", menuOptions[currentSelection].text);
        ch = _getch();
        if (ch == KEY_ESC) {
            currentSelection = MENU_BACK;
            break;
        }
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
                currentSelection += (currentSelection >= 1) ? 1 : 2;
                break;
            case KEY_A:
            case KEY_LEFT:
                currentSelection -= (currentSelection >= 1) ? 2 : 1;
                break;
            default:
                continue;
        }
        currentSelection = (currentSelection + 3) % 3;
        setCursorVerticalHorizontalPosition(38 + menuOptions[previousSelection].row * 2, menuOptions[previousSelection].consoleColumn);
        printf("  %s", menuOptions[previousSelection].text);
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || !canAffordSelection || !unitHasValidCells);
    if (currentSelection == MENU_BACK) {
        for (int i = 0; i < 3; i++) {
            List_clear(&unitValidCells[i]);
        }
        return currentSelection;
    }
    const EntityType selectedUnit = currentSelection + 6;
    const size_t bufSize = snprintf(NULL, 0, "Spawn %s...", menuOptions[currentSelection].text) + 1;
    char *boxTitle = malloc(bufSize);
    (void)snprintf(boxTitle, bufSize, "Spawn %s...", menuOptions[currentSelection].text);
    drawActionsMenu(boxTitle, MENU_FOOTER_CANCEL, true);
    free(boxTitle);
    setCursorVerticalHorizontalPosition(40, 12);
    printf("Navigate the map with the arrow keys or WASD to choose where you want to spawn the unit.");
    setCursorVerticalHorizontalPosition(42, 12);
    printf("Press [SPACE] or [ENTER] to confirm the location.");
    Int16Vector2 selectedCellCoord = gameDataEx->currentPlayerBaseCoord;
    GameBoardCell* selectedCell = getSelectedGameBoardCell(gameDataEx, &selectedCellCoord, unitValidCells[currentSelection]);
    for (int i = 0; i < 3; i++) {
        List_clear(&unitValidCells[i]);
    }
    if (selectedCell == NULL) return openSpawnUnitMenu(gameDataEx, currentSelection); // If it's NULL, the user must have pressed ESC.
    createEntity(selectedCell, gameDataEx->currentPlayer, selectedUnit);
    setCursorAtCellCoord(selectedCellCoord);
    printGameCell(&gameDataEx->gameData, selectedCell, false);
    resetBackgroundColor();
    return currentSelection;
}

Node* getNextInList(const List list, Node* currentNode) {
    Node *node = currentNode ? currentNode : list.tail;
    if (node == list.tail) {
        node = list.head;
    } else {
        node = node->next;
    }
    return node;
}

Node* getPreviousInList(const List list, Node* currentNode) {
    Node *node = currentNode ? currentNode : list.head;
    if (node == list.head) {
        node = list.tail;
    } else {
        node = node->previous;
    }
    return node;
}

Int16Vector2* makeEntitySelection(const GameDataExtended* gameDataEx, const bool isUnitSelection) {
    const List list = isUnitSelection ? gameDataEx->currentPlayerUnitsList : gameDataEx->currentPlayerBuildingsList;
    Node *node = getNextInList(list, NULL);
    if (node == NULL) return NULL;
    printEntityList(gameDataEx, isUnitSelection ? 2 : 1);
    drawActionsMenu(isUnitSelection ? "Select Unit..." : "Select Building...", MENU_FOOTER_CANCEL, true);
    Int16Vector2 *currentCellCoord;
    int ch;
    do {
        currentCellCoord = node->data;
        const GameBoardCell *currentCell = &gameDataEx->gameData.board[currentCellCoord->y][currentCellCoord->x];
        setCursorAtCellCoord(*currentCellCoord);
        enableBlinking();
        printGameCell(&gameDataEx->gameData, currentCell, true);
        resetBackgroundColor();
        disableBlinking();
        clearActionsMenu();
        setCursorVerticalHorizontalPosition(40, 50);
        printf("Use the [↑]/[↓] keys to navigate your %s.", isUnitSelection ? "units" : "buildings");
        setCursorVerticalHorizontalPosition(42, 50);
        printf("Press [SPACE] or [ENTER] to confirm the selection.");
        setCursorVerticalHorizontalPosition(40, 12);
        printf("%s", getEntityName(currentCell->entityType, gameDataEx->gameData.players[currentCell->owner].isMordor));
        setCursorVerticalHorizontalPosition(40, currentCellCoord->y > 9 ? 38 : 39);
        printf("%c-%d", 'A' + currentCellCoord->x, currentCellCoord->y);
        const uint16_t maxHealth = getEntityInfo(currentCell->entityType).maxHealth;
        const double remainingHPFactor = (double)currentCell->health / maxHealth;
        const uint16_t remainingHealthBarWidth = (uint16_t)(30 * remainingHPFactor);
        const uint16_t lostHealthBarWidth = 30 - remainingHealthBarWidth;
        if (remainingHPFactor <= 1.0/3) {
            setForegroundColor(RED);
        } else if (remainingHPFactor <= 2.0/3) {
            setForegroundColor(YELLOW);
        } else {
            setForegroundColor(GREEN);
        }
        setCursorVerticalHorizontalPosition(42, 12);
        printf("HP %d/%d", currentCell->health, maxHealth);
        setCursorVerticalHorizontalPosition(41, 12);
        for (uint16_t i = 0; i < remainingHealthBarWidth; ++i) {
            printf("▃");
        }
        if (lostHealthBarWidth) {
            setForegroundColor(DARK_GRAY);
            for (uint16_t i = 0; i < lostHealthBarWidth; ++i) {
                printf("▃");
            }
        }
        resetForegroundColor();
        const Int16Vector2 *previousCoord = currentCellCoord;
        const GameBoardCell *previousCell = currentCell;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        switch (ch) {
            case KEY_W:
            case KEY_UP:
                node = getPreviousInList(list, node);
                break;
            case KEY_S:
            case KEY_DOWN:
                node = getNextInList(list, node);
                break;
        }
        setCursorAtCellCoord(*previousCoord);
        printGameCell(&gameDataEx->gameData, previousCell, false);
        resetBackgroundColor();
        if (ch == KEY_ESC) {
            return NULL;
        }
    } while (ch != KEY_ENTER && ch != KEY_SPACE);
    return currentCellCoord;
}

int32_t openSelectBuildingMenu(GameDataExtended* gameDataEx) {
    const Int16Vector2 *selectedCellCoord = makeEntitySelection(gameDataEx, false);
    if (selectedCellCoord == NULL) return MENU_BACK;
    GameBoardCell *cell = &gameDataEx->gameData.board[selectedCellCoord->y][selectedCellCoord->x];
    const EntityInfo entityInfo = getEntityInfo(cell->entityType);
    drawActionsMenu("Building Actions", MENU_FOOTER_GO_BACK, false);
    setCursorVerticalHorizontalPosition(40, 50);
    clearFromCursorForward(57);
    setCursorVerticalHorizontalPosition(42, 50);
    clearFromCursorForward(57);
    char *repairText;
    uint16_t repairCost = 0;
    if (cell->entityType == BASE) {
        repairText = "Repair (Unavailable)";
    } else if (cell->health < entityInfo.maxHealth) {
        repairText = "Repair";
        repairCost = max((uint16_t)(entityInfo.spawnCost * (1.0 - (double)cell->health / entityInfo.maxHealth)), 1);
    } else {
        repairText = "Repair (Already Max HP)";
    }
    const int32_t selection = makeActionMenu(gameDataEx, (ActionMenuOption[]){
        {repairText, repairCost, repairCost == 0, 1, 50},
        {"Destroy", 0, false, 2, 50}
    }, 2, NULL);
    switch (selection) {
        case 0:
            cell->health = (int16_t)entityInfo.maxHealth;
            gameDataEx->currentPlayer->coins -= repairCost;
            break;
        case 1:
            if (cell->entityType == BASE) {
                /* TODO: Implement a function to make a player "lose" and end the game if only one player remaining.
                 * For future-proofing, if we were to support more than two players, we should make a function that
                 * will clear all of the losing player's entities, giving way to the remaining players.
                 *If we only have two players, we do not need to clear all entities, but the rest remains true.
                 * We should check somewhere if there is only one player still "alive", if true, then it is game over.
                 */
                break;
            }
            cell->entityType = EMPTY_CELL;
            cell->health = 0;
            cell->owner = NO_OWNER;
            setCursorAtCellCoord(*selectedCellCoord);
            printGameCell(&gameDataEx->gameData, cell, false);
            resetBackgroundColor();
            break;
    }
    return selection;
}

int32_t openSelectUnitMenu(GameDataExtended* gameDataEx) {
    const Int16Vector2 *selectedCellCoord = makeEntitySelection(gameDataEx, true);
    if (selectedCellCoord == NULL) return MENU_BACK;
    
    return MENU_BACK;
}

int32_t openMainActionsMenu(const GameDataExtended* gameDataEx, int32_t* currentSelection) {
    drawActionsMenu("Actions", NULL, true);
    return makeActionMenu(gameDataEx, (ActionMenuOption[]){
        {"Build", 0, false, 1, 10},
        {"Spawn Unit", 0, false, 2, 10},
        {"Select Building", 0, gameDataEx->currentPlayerBuildingsList.length <= 1, 1, 42},
        {"Select Unit", 0, !gameDataEx->currentPlayerUnitsList.length, 2, 42},
        {"End Turn", 0, false, 1, 80},
        {"Exit Game", 0, false, 2, 80}
    }, 6, currentSelection);
}

void Game(GameDataExtended* gameDataEx) {
    int32_t currentSelection = 0;
    gameDataEx->currentPlayer = &gameDataEx->gameData.players[gameDataEx->gameData.currentPlayerTurn];
    gameDataEx->currentPlayerBaseCoord = getPlayerBaseCoordinate(&gameDataEx->gameData, gameDataEx->gameData.currentPlayerTurn);
    printGameBoard(&gameDataEx->gameData);
    bool shouldRefreshEntityLists = true;
    while (!gameDataEx->gameData.isGameOver) {
        if (shouldRefreshEntityLists) {
            refreshCurrentPlayerEntityLists(gameDataEx);
            shouldRefreshEntityLists = false;
        }
        printEntityList(gameDataEx, 0);
        printTurnInfoBox(gameDataEx);
        while (true) {
            const int32_t action = openMainActionsMenu(gameDataEx, &currentSelection);
            if (action == 0) {
                shouldRefreshEntityLists = openCreateBuildingMenu(gameDataEx, 0) != MENU_BACK;
                break;
            } else if (action == 1) {
                shouldRefreshEntityLists = openSpawnUnitMenu(gameDataEx, 0) != MENU_BACK;
                break;
            } else if (action == 2) {
                shouldRefreshEntityLists = openSelectBuildingMenu(gameDataEx) != MENU_BACK;
                break;
            } else if (action == 3) {
                shouldRefreshEntityLists = openSelectUnitMenu(gameDataEx) != MENU_BACK;
                break;
            } else if (action == 4) {
                advanceTurn(gameDataEx);
                currentSelection = 0;
                break;
            } else {
                const int32_t exitResult = openExitGameMenu();
                switch (exitResult) {
                    case 0:
                    case 1:
                        // saveGame(gameDataEx);
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
        // if (gameDataEx->currentPlayer.coins <= 0) advanceTurn(gameDataEx);
    }
}

void createBoardFromMapFile(char* mapFileName, GameData* gameData) {
    FILE *fp = fopen(mapFileName, "r");
    if (fp == NULL) return;
    uint8_t player = 0;
    for (uint16_t y = 0; y < 17; y++) {
        for (uint16_t x = 0; x < 26; x++) {
            const int ch = fgetc(fp);
            if (ch == EOF) {
                y = 17;
                break;
            }
            switch (ch) {
                default:
                case 'P':
                    gameData->board[y][x].terrainType = PLAIN;
                    break;
                case 'F':
                    gameData->board[y][x].terrainType = FOREST;
                    break;
                case 'M':
                    gameData->board[y][x].terrainType = MOUNTAIN;
                    break;
                case 'R':
                    gameData->board[y][x].terrainType = RIVER;
                    break;
                case 'W':
                    gameData->board[y][x].terrainType = WATER;
                    break;
                case 'B':
                    gameData->board[y][x].terrainType = BRIDGE;
                    break;
                case 'S':
                    gameData->board[y][x].terrainType = SNOW;
                    break;
                case 'L':
                    gameData->board[y][x].terrainType = LAVA;
                    break;
                case 'T':
                    gameData->board[y][x].terrainType = BASALT;
                    break;
                case '#':
                    createEntity(&gameData->board[y][--x], &gameData->players[player++], BASE);
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
    GameDataExtended gameDataEx = {
        {
            time(NULL),
            0,
            {
                {0, "Player 1", 100, player1IsMordor, false },
                {1, "CPU", 100, !player1IsMordor, true }
            },
            0,
            false,
            {{{PLAIN, EMPTY_CELL, NO_OWNER, 0}}}
        },
        NULL,
        {0, 0},
        List_init(),
        List_init()
    };
    createBoardFromMapFile(mapFile, &gameDataEx.gameData);
    memcpy(gameDataEx.gameData.players[0].name, playerName, 20);
    Game(&gameDataEx);
}

void startNewMultiplayerGame(char* mapFile, const char* player1Name, const char* player2Name, const bool player1IsMordor) {
    GameDataExtended gameDataEx = {
        {
            time(NULL),
            0,
            {
                {0, "Player 1", 100, player1IsMordor, false },
                {1, "Player 2", 100, !player1IsMordor, false }
            },
            0,
            false,
            {{{PLAIN, EMPTY_CELL, NO_OWNER, 0}}}
        },
        NULL,
        {0, 0},
        List_init(),
        List_init()
    };
    createBoardFromMapFile(mapFile, &gameDataEx.gameData);
    memcpy(gameDataEx.gameData.players[0].name, player1Name, 20);
    memcpy(gameDataEx.gameData.players[1].name, player2Name, 20);
    Game(&gameDataEx);
}
