#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
    5, 7, 10,
    true
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

bool saveGame(GameData* gameData, const uint8_t saveSlot) {
    const size_t bufSize = snprintf(NULL, 0, "Slot_%02d.savegame", saveSlot) + 1;
    char *filename = malloc(bufSize);
    (void)snprintf(filename, bufSize, "Slot_%02d.savegame", saveSlot);
    FILE* file = fopen(filename, "wb");
    if (file == NULL) return false;
    gameData->lastSaveTimestamp = time(NULL);
    // Write the fixed-size part of GameData
    (void)fwrite(gameData, sizeof(GameData), 1, file);
    // Write the players data
    (void)fwrite(gameData->players, sizeof(Player), gameData->nPlayers, file);
    (void)fclose(file);
    free(filename);
    return true;
}

bool loadGame(GameData** gameData, const uint8_t saveSlot) {
    const size_t bufSize = snprintf(NULL, 0, "Slot_%02d.savegame", saveSlot) + 1;
    char *filename = malloc(bufSize);
    (void)snprintf(filename, bufSize, "Slot_%02d.savegame", saveSlot);
    FILE* file = fopen(filename, "rb");
    if (file == NULL) return false;
    
    // Read the fixed-size part of GameData
    GameData tempGameData;
    (void)fread(&tempGameData, sizeof(GameData), 1, file);
    
    // Allocate memory for the full GameData including the players
    *gameData = malloc(sizeof(GameData) + tempGameData.nPlayers * sizeof(Player));
    memcpy(*gameData, &tempGameData, sizeof(GameData));
    
    // Read the players data
    (void)fread((*gameData)->players, sizeof(Player), (*gameData)->nPlayers, file);
    
    (void)fclose(file);
    free(filename);
    return true;
}

List createInt16Vector2List(void) {
    return List_init(&compareInt16Vector2);
}

void clearEntityListBox(void) {
    for (uint16_t i = 0; i < 33; ++i) {
        setCursorVerticalHorizontalPosition(3 + i, 109);
        clearFromCursorForward(24);
    }
}

void clearTurnInfoBox(void) {
    for (uint16_t i = 0; i < 7; ++i) {
        setCursorVerticalHorizontalPosition(38 + i, 109);
        clearFromCursorForward(24);
    }
}

void clearActionsMenu(void) {
    for (uint16_t i = 0; i < 7; ++i) {
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
    for (int16_t y = 0; y < 17; ++y) {
        for (int16_t x = 0; x < 26; ++x) {
            if (gameData->board[y][x].entityType == BASE && gameData->board[y][x].owner == playerId) {
                return (Int16Vector2){x, y};
            }
        }
    }
    return (Int16Vector2){0, 0};
}

uint32_t getTerrainBackgroundColor(const TerrainType terrain) {
    switch (terrain) {
        case PLAIN:    return C_PLAIN;
        case FOREST:   return C_FOREST;
        case MOUNTAIN: return C_MOUNTAIN;
        case RIVER:    return C_RIVER;
        case WATER:    return C_WATER;
        case BRIDGE:   return C_BRIDGE;
        case SNOW:     return C_SNOW;
        case LAVA:     return C_LAVA;
        case BASALT:   return C_BASALT;
    }
    return BLACK;
}

double getTerrainMovementCostModifier(const TerrainType terrainType) {
    switch (terrainType) {
        default: case PLAIN: case BASALT: return 1;
        case FOREST:                      return 1.25;
        case MOUNTAIN:                    return 2;
        case WATER:                       return 1.75;
        case BRIDGE:                      return 1.15;
        case SNOW:                        return 1.5;
    }
}

double getAttackerTerrainAttackPowerModifier(const TerrainType terrainType) {
    switch (terrainType) {
        default: case PLAIN: case BASALT: return 1;
        case FOREST:                      return 1.2;
        case MOUNTAIN:                    return 1.3;
        case WATER:                       return 0.75;
        case BRIDGE:                      return 0.9;
        case SNOW:                        return 0.85;
    }
}

double getVictimTerrainAttackPowerModifier(const TerrainType terrainType) {
    switch (terrainType) {
        default: case PLAIN: case BASALT:   return 1;
        case FOREST:                        return 0.75;
        case MOUNTAIN:                      return 0.5;
        case WATER: case BRIDGE: case SNOW: return 1.15;
    }
}

const char* getTerrainName(const TerrainType terrainType) {
    switch (terrainType) {
        case PLAIN:    return "Plains";
        case FOREST:   return "Forest";
        case MOUNTAIN: return "Mountains";
        case RIVER:    return "River";
        case WATER:    return "Water";
        case BRIDGE:   return "Bridge";
        case SNOW:     return "Snow";
        case LAVA:     return "Lava";
        case BASALT:   return "Basalt";
    }
    return NULL;
}

const char* getEntityName(const EntityType entityType, const bool ownerIsMordor) {
    switch (entityType) {
        case BASE:      return "Base";
        case MINES:     return "Mines";
        case BARRACKS:  return "Barracks";
        case STABLES:   return "Stables";
        case ARMOURY:   return "Armoury";
        case INFANTRY:  return ownerIsMordor ? "Orc Warriors" : "Citadel Guards";
        case CAVALRY:   return ownerIsMordor ? "Wargs" : "Swan-Knights";
        case ARTILLERY: return ownerIsMordor ? "Siege Towers" : "Trebuchets";
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

bool isTerrainPassable(const TerrainType terrain) {
    switch (terrain) {
        case RIVER: case LAVA: return false;
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
    const GameBoardCell* cell = &gameDataEx->gameData->board[cellCoord.y][cellCoord.x];
    if (cell->entityType != EMPTY_CELL) return false;
    switch (cell->terrainType) {
        case BRIDGE: case RIVER: case WATER: case LAVA:
            return false;
        default:
            return true;
    }
}

List getValidCellsToBuild(const GameDataExtended* gameDataEx) {
    const GameData *gameData = gameDataEx->gameData;
    List validCells = createInt16Vector2List();
    for (int16_t y = 0; y < 17; ++y) {
        for (int16_t x = 0; x < 26; ++x) {
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

List getValidCellsToSpawnUnit(const GameDataExtended* gameDataEx, const EntityType unit) {
    List validCells = createInt16Vector2List();
    EntityType requiredBuilding;
    switch (unit) {
        case INFANTRY:  requiredBuilding = BARRACKS; break;
        case CAVALRY:   requiredBuilding = STABLES;  break;
        case ARTILLERY: requiredBuilding = ARMOURY;  break;
        default: return validCells;
    }
    const Node *buildingNode = gameDataEx->currentPlayerBuildingsList.head;
    while (buildingNode) {
        const Int16Vector2 *buildingCoord = buildingNode->data;
        const GameBoardCell *buildingCell = &gameDataEx->gameData->board[buildingCoord->y][buildingCoord->x];
        if (buildingCell->entityType == requiredBuilding) {
            // Add all valid adjacent cells to the validCells list.
            const uint16_t minX = max(buildingCoord->x - 1, 0);
            const uint16_t maxX = min(buildingCoord->x + 1, 25);
            const uint16_t minY = max(buildingCoord->y - 1, 0);
            const uint16_t maxY = min(buildingCoord->y + 1, 16);
            for (uint16_t y = minY; y <= maxY; ++y) {
                for (uint16_t x = minX; x <= maxX; ++x) {
                    if (buildingCoord->y == y && buildingCoord->x == x) continue;
                    if (!canUnitStepOnCell(&gameDataEx->gameData->board[y][x])) continue;
                    Int16Vector2 *c = malloc(sizeof(Int16Vector2));
                    c->x = (int16_t)x;
                    c->y = (int16_t)y;
                    List_append(&validCells, c);
                }
            }
        }
        buildingNode = buildingNode->next;
    }
    return validCells;
}

List getValidCellsToMoveUnit(const GameDataExtended* gameDataEx, const Int16Vector2 unitCoord) {
    List validCellsToMoveTo = createInt16Vector2List();
    const uint16_t minX = max(unitCoord.x - 1, 0);
    const uint16_t maxX = min(unitCoord.x + 1, 25);
    const uint16_t minY = max(unitCoord.y - 1, 0);
    const uint16_t maxY = min(unitCoord.y + 1, 16);
    for (uint16_t y = minY; y <= maxY; ++y) {
        for (uint16_t x = minX; x <= maxX; ++x) {
            if (unitCoord.y == y && unitCoord.x == x) continue;
            if (!canUnitStepOnCell(&gameDataEx->gameData->board[y][x])) continue;
            Int16Vector2 *c = malloc(sizeof(Int16Vector2));
            c->x = (int16_t)x;
            c->y = (int16_t)y;
            List_append(&validCellsToMoveTo, c);
        }
    }
    return validCellsToMoveTo;
}

List getEnemyEntitiesInRange(const GameDataExtended* gameDataEx, const Int16Vector2 unitCoord) {
    const GameBoardCell *unit = &gameDataEx->gameData->board[unitCoord.y][unitCoord.x];
    const uint8_t unitRange = unit->entityType == ARTILLERY ? 3 : 1;
    List enemiesInRange = createInt16Vector2List();
    const uint16_t minX = max(unitCoord.x - unitRange, 0);
    const uint16_t maxX = min(unitCoord.x + unitRange, 25);
    const uint16_t minY = max(unitCoord.y - unitRange, 0);
    const uint16_t maxY = min(unitCoord.y + unitRange, 16);
    for (uint16_t y = minY; y <= maxY; ++y) {
        for (uint16_t x = minX; x <= maxX; ++x) {
            const GameBoardCell *cell = &gameDataEx->gameData->board[y][x];
            if (cell->entityType == EMPTY_CELL || cell->owner == unit->owner) continue;
            Int16Vector2 *c = malloc(sizeof(Int16Vector2));
            c->x = (int16_t)x;
            c->y = (int16_t)y;
            List_append(&enemiesInRange, c);
        }
    }
    return enemiesInRange;
}

uint16_t getNumberOfMines(const GameData* gameData, const uint8_t player) {
    uint16_t minesCount = 0;
    for (uint16_t y = 0; y < 17; ++y) {
        for (uint16_t x = 0; x < 26; ++x) {
            const GameBoardCell cell = gameData->board[y][x];
            if (cell.owner == player && cell.entityType == MINES) {
                ++minesCount;
            }
        }
    }
    return minesCount;
}

void refreshCurrentPlayerEntityLists(GameDataExtended* gameDataEx) {
    List_clear(&gameDataEx->currentPlayerBuildingsList);
    List_clear(&gameDataEx->currentPlayerUnitsList);
    Int16Vector2 *baseCoord = malloc(sizeof(Int16Vector2));
    *baseCoord = gameDataEx->currentPlayerBaseCoord;
    List_append(&gameDataEx->currentPlayerBuildingsList, baseCoord);
    for (int16_t y = 0; y < 17; ++y) {
        for (int16_t x = 0; x < 26; ++x) {
            const GameBoardCell *cell = &gameDataEx->gameData->board[y][x];
            if (cell->entityType <= BASE || cell->owner != gameDataEx->gameData->currentPlayerTurn) continue;
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
    assert(cell->entityType == EMPTY_CELL && entityType != EMPTY_CELL);
    const EntityInfo entityInfo = getEntityInfo(entityType);
    if (player->coins < entityInfo.spawnCost) return false;
    cell->owner = player->id;
    cell->entityType = entityType;
    cell->health = (int16_t)entityInfo.maxHealth;
    player->coins -= entityInfo.spawnCost;
    return true;
}

void advanceTurn(GameDataExtended* gameDataEx) {
    // Change to next player.
    gameDataEx->gameData->currentPlayerTurn = ++gameDataEx->gameData->currentPlayerTurn % gameDataEx->gameData->nPlayers;
    gameDataEx->currentPlayer = &gameDataEx->gameData->players[gameDataEx->gameData->currentPlayerTurn];
    gameDataEx->currentPlayerBaseCoord = getPlayerBaseCoordinate(gameDataEx->gameData, gameDataEx->gameData->currentPlayerTurn);
    const Node* node = gameDataEx->currentPlayerUnitsList.head;
    while (node) {
        const Int16Vector2 *unitCoord = node->data;
        GameBoardCell *unitCell = &gameDataEx->gameData->board[unitCoord->y][unitCoord->x];
        unitCell->hasAttackedThisRound = false;
        node = node->next;
    }
    refreshCurrentPlayerEntityLists(gameDataEx);
    if (gameDataEx->gameData->currentPlayerTurn == 0) ++gameDataEx->gameData->currentRound;
    // Add Castar Coins based on the player's number of mines.
    const uint16_t playerMinesCount = getNumberOfMines(gameDataEx->gameData, gameDataEx->gameData->currentPlayerTurn);
    if (playerMinesCount) gameDataEx->currentPlayer->coins += playerMinesCount * gameDataEx->gameSettings->MINE_INCOME;
}

void setCursorAtCellCoord(const Int16Vector2 cellCoord) {
    setCursorVerticalHorizontalPosition(1 + 2 * (cellCoord.y + 1), 4 * (cellCoord.x + 1));
}

const char* getCellStrRepr(const GameData* gameData, const GameBoardCell* cell) {
    switch (cell->entityType) {
        case BASE:       return gameData->players[cell->owner].isMordor ? "MMM" : "GGG";
        case MINES:      return " M ";
        case BARRACKS:   return " B ";
        case STABLES:    return " S ";
        case ARMOURY:    return " F ";
        case INFANTRY:   return " I ";
        case CAVALRY:    return " C ";
        case ARTILLERY:  return " A ";
        case EMPTY_CELL: return "   ";
    }
    return "   ";
}

void printGameCell(const GameData* gameData, const GameBoardCell* cell, const bool darken) {
    // If the cell is not empty but it has no owner, something went wrong.
    assert(!(cell->entityType != EMPTY_CELL && cell->owner == NO_OWNER));
    const uint32_t terrainColor = getTerrainBackgroundColor(cell->terrainType);
    setBackgroundColor(darken ? modifyColorBrightness(terrainColor, 0.7f) : terrainColor);
    if (cell->owner != NO_OWNER) {
        setForegroundColor(gameData->players[cell->owner].isMordor ? C_MORDOR : C_GONDOR);
    }
    printf("%s", getCellStrRepr(gameData, cell));
    resetForegroundColor();
}

uint16_t getMovementCost(const GameData* gameData, const Int16Vector2 srcCoord, const Int16Vector2 dstCoord) {
    const GameBoardCell *srcCell = &gameData->board[srcCoord.y][srcCoord.x];
    const GameBoardCell *dstCell = &gameData->board[dstCoord.y][dstCoord.x];
    return (uint16_t)(round(
        getOctileDistance(srcCoord, dstCoord) * getEntityInfo(srcCell->entityType).moveCost
        * getTerrainMovementCostModifier(dstCell->terrainType)
    ));
}

uint16_t getAttackPower(const GameData* gameData, const Int16Vector2 attackerCoord, const Int16Vector2 victimCoord) {
    const GameBoardCell *attackerCell = &gameData->board[attackerCoord.y][attackerCoord.x];
    const GameBoardCell *victimCell = &gameData->board[victimCoord.y][victimCoord.x];
    const EntityInfo attackerInfo = getEntityInfo(attackerCell->entityType);
    return (uint16_t)(round((double)attackerInfo.attackPower
        * getAttackerTerrainAttackPowerModifier(attackerCell->terrainType)
        * getVictimTerrainAttackPowerModifier(victimCell->terrainType)
    ));
}

bool moveUnit(GameData* gameData, const Int16Vector2 srcCoord, const Int16Vector2 dstCoord) {
    GameBoardCell *const srcCell = &gameData->board[srcCoord.y][srcCoord.x];
    GameBoardCell *const dstCell = &gameData->board[dstCoord.y][dstCoord.x];
    Player *unitOwner = &gameData->players[srcCell->owner];
    if (dstCell->entityType != EMPTY_CELL) return false;
    const uint16_t movementCost = getMovementCost(gameData, srcCoord, dstCoord);
    if (unitOwner->coins < movementCost) return false;
    unitOwner->coins -= movementCost;
    *dstCell = (GameBoardCell){dstCell->terrainType, srcCell->entityType, srcCell->owner, srcCell->health, srcCell->hasAttackedThisRound};
    *srcCell = (GameBoardCell){srcCell->terrainType, EMPTY_CELL, NO_OWNER, 0, false};
    setCursorAtCellCoord(srcCoord);
    printGameCell(gameData, srcCell, false);
    resetBackgroundColor();
    setCursorAtCellCoord(dstCoord);
    printGameCell(gameData, dstCell, false);
    resetBackgroundColor();
    return true;
}

void processPlayerLoss(GameData* gameData, const uint8_t playerId) {
    gameData->players[playerId].isAlive = false;
    uint8_t nPlayersAlive = 0;
    for (uint8_t i = 0; i < gameData->nPlayers; ++i) {
        if (gameData->players[i].isAlive) ++nPlayersAlive;
    }
    if (nPlayersAlive <= 1) gameData->isGameOver = true;
}

/**
 * @return True if the attacked entity was destroyed, False if otherwise.
 */
bool performAttack(GameData* gameData, const Int16Vector2 attackerCoord, const Int16Vector2 victimCoord) {
    GameBoardCell *attackerCell = &gameData->board[attackerCoord.y][attackerCoord.x];
    const uint8_t attackerRange = attackerCell->entityType == ARTILLERY ? 3 : 1;
    if (getChebyshevDistance(attackerCoord, victimCoord) > attackerRange) return false;
    attackerCell->hasAttackedThisRound = true;
    GameBoardCell *victimCell = &gameData->board[victimCoord.y][victimCoord.x];
    const uint16_t attackPower = getAttackPower(gameData, attackerCoord, victimCoord);
    if (victimCell->health <= attackPower) {
        if (victimCell->entityType == BASE) {
            victimCell->health = 0;
            processPlayerLoss(gameData, victimCell->owner);
        } else {
            *victimCell = (GameBoardCell){victimCell->terrainType, EMPTY_CELL, NO_OWNER, 0, false};
            setCursorAtCellCoord(victimCoord);
            printGameCell(gameData, victimCell, false);
            resetBackgroundColor();
        }
        return true;
    }
    victimCell->health = (int16_t)(victimCell->health - attackPower);
    return false;
}

void printGameBoard(const GameData* gameData) {
    resetTextDecoration();
    printf("   ");
    for (int i = 'A'; i <= 'Z'; ++i) {
        printf(" %c  ", i);
    }
    printf("\n  ╭");
    for (int i = 0; i < 25; ++i) {
        printf("────");
    }
    printf("───╮\n");
    for (int y = 0; y < 16; ++y) {
        if (y < 10) printf(" %d│", y);
        else printf("%d│", y);
        for (int x = 0; x < 25; ++x) {
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
        for (int x = 0; x < 25; ++x) {
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
    for (int x = 0; x < 25; ++x) {
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
    for (int i = 0; i < 25; ++i) {
        printf("────");
    }
    printf("───╯\n");
}

void printTurnInfoBox(const GameDataExtended* gameDataEx) {
    clearTurnInfoBox();
    setCursorVerticalHorizontalPosition(37, 109);
    const size_t bufSize = snprintf(NULL, 0, "Round %d", gameDataEx->gameData->currentRound + 1) + 1;
    char *roundInfo = malloc(bufSize);
    (void)snprintf(roundInfo, bufSize, "Round %d", gameDataEx->gameData->currentRound + 1);
    drawBoxWithTitleAndFooter(roundInfo, NULL, 26, 9, drawRoundedBox);
    free(roundInfo);
    setCursorVerticalHorizontalPosition(39, 118);
    printf("Player %d", gameDataEx->gameData->currentPlayerTurn + 1);
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
        const GameBoardCell *cell = &gameDataEx->gameData->board[cellCoord->y][cellCoord->x];
        if (count < 16) { // The box can only fit a maximum of 16 elements.
            setCursorVerticalHorizontalPosition(4 + count * 2, 111);
            printf("%s", getEntityName(cell->entityType, gameDataEx->gameData->players[cell->owner].isMordor));
            setCursorVerticalHorizontalPosition(4 + count * 2, cellCoord->y > 9 ? 129 : 130);
            printf("%c-%d", 'A' + cellCoord->x, cellCoord->y);
            setCursorVerticalHorizontalPosition(5 + count * 2, 111);
            const uint16_t maxHealth = getEntityInfo(cell->entityType).maxHealth;
            const double remainingHPFactor = (double)cell->health / maxHealth;
            const uint16_t remainingHealthBarWidth = (uint16_t)(remainingHPFactor * 14);
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
    const Node *e = validCells.head;
    while (e) {
        const Int16Vector2 *eData = e->data;
        if (eData->x == cellCoord.x && eData->y == cellCoord.y) return true;
        e = e->next;
    }
    return false;
}

void drawValidCells(const GameData* gameData, const List validCells, const bool darken) {
    const Node *e = validCells.head;
    while (e) {
        const Int16Vector2 *eData = e->data;
        setCursorAtCellCoord(*eData);
        printGameCell(gameData, &gameData->board[eData->y][eData->x], darken);
        e = e->next;
    }
    resetBackgroundColor();
}

GameBoardCell* getSelectedGameBoardCell(GameDataExtended* gameDataEx, Int16Vector2* currentCoord, const List validCells) {
    drawValidCells(gameDataEx->gameData, validCells, true);
    GameBoardCell *currentCell;
    bool isSelectedCellValid;
    int ch;
    do {
        currentCell = &gameDataEx->gameData->board[currentCoord->y][currentCoord->x];
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
            case KEY_W: case KEY_UP:    --currentCoord->y; break;
            case KEY_S: case KEY_DOWN:  ++currentCoord->y; break;
            case KEY_D: case KEY_RIGHT: ++currentCoord->x; break;
            case KEY_A: case KEY_LEFT:  --currentCoord->x; break;
            case KEY_ENTER: case KEY_SPACE: case KEY_ESC:  break;
            default: continue;
        }
        setCursorAtCellCoord(previousCoord);
        // Here `currentCell` still refers to the previously selected cell.
        printGameCell(gameDataEx->gameData, currentCell, isSelectedCellValid);
        resetBackgroundColor();
        if (ch == KEY_ESC) {
            currentCell = NULL;
            break;
        }
        currentCoord->x = (int16_t)((currentCoord->x + 26) % 26);
        currentCoord->y = (int16_t)((currentCoord->y + 17) % 17);
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || !isSelectedCellValid);
    drawValidCells(gameDataEx->gameData, validCells, false);
    return currentCell;
}

int32_t makeActionMenu(
    const GameDataExtended* gameDataEx,
    ActionMenuOption menuOptions[],
    const uint8_t numberOfOptions,
    int32_t* selection
) {
    int32_t currentSelection = selection == NULL ? 0 : *selection;
    for (uint8_t i = 0; i < numberOfOptions; ++i) {
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
            case KEY_W: case KEY_UP:    --currentSelection;    break;
            case KEY_S: case KEY_DOWN:  ++currentSelection;    break;
            case KEY_D: case KEY_RIGHT: currentSelection += 2; break;
            case KEY_A: case KEY_LEFT:  currentSelection -= 2; break;
            case KEY_ENTER: case KEY_SPACE: case KEY_ESC:      break;
            default: continue;
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
    List validCells = getValidCellsToBuild(gameDataEx);
    if (!validCells.length) {
        setCursorVerticalHorizontalPosition(41, 12);
        setForegroundColor(RED);
        printf("No land space available to build on.");
        resetForegroundColor();
        while (_getch() != KEY_ESC) {}
        return MENU_BACK;
    }
    ActionMenuOption actionMenu[] = {
        {"Mines", gameDataEx->gameSettings->MINES_COST, false, 1, 10},
        {"Barracks", gameDataEx->gameSettings->BARRACKS_COST, false, 2, 10},
        {"Stables", gameDataEx->gameSettings->STABLES_COST, false, 1, 42},
        {"Armoury", gameDataEx->gameSettings->ARMOURY_COST, false, 2, 42}
    };
    const int32_t selection = makeActionMenu(gameDataEx, actionMenu, 4, &currentSelection);
    if (selection == MENU_BACK) return selection;
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
    printGameCell(gameDataEx->gameData, selectedCell, false);
    resetBackgroundColor();
    return selection;
}

int32_t openSpawnUnitMenu(GameDataExtended* gameDataEx, int32_t currentSelection) {
    const bool playerIsMordor = gameDataEx->currentPlayer->isMordor;
    const ActionMenuOption menuOptions[] = {
        {playerIsMordor ? "Orc Warriors (Infantry)" : "Citadel Guards (Infantry)", gameDataEx->gameSettings->INFANTRY_SPAWN_COST, false, 1, 10},
        {playerIsMordor ? "Wargs (Cavalry)" : "Swan-Knights (Cavalry)", gameDataEx->gameSettings->CAVALRY_SPAWN_COST, false, 2, 10},
        {playerIsMordor ? "Siege Towers (Artillery)" : "Trebuchets (Artillery)", gameDataEx->gameSettings->ARTILLERY_SPAWN_COST, false, 1, 60}
    };
    const char *buildingRequirements[] = {"Barracks", "Stables", "Armoury"};
    List unitValidCells[] = {
        getValidCellsToSpawnUnit(gameDataEx, INFANTRY),
        getValidCellsToSpawnUnit(gameDataEx, CAVALRY),
        getValidCellsToSpawnUnit(gameDataEx, ARTILLERY)
    };
    drawActionsMenu("Spawn Unit...", MENU_FOOTER_GO_BACK, true);
    for (uint8_t i = 0; i < 3; ++i) {
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
            case KEY_W: case KEY_UP:    --currentSelection; break;
            case KEY_S: case KEY_DOWN:  ++currentSelection; break;
            case KEY_D: case KEY_RIGHT: currentSelection += (currentSelection >= 1) ? 1 : 2; break;
            case KEY_A: case KEY_LEFT:  currentSelection -= (currentSelection >= 1) ? 2 : 1; break;
            case KEY_ENTER: case KEY_SPACE: case KEY_ESC: break;
            default: continue;
        }
        currentSelection = (currentSelection + 3) % 3;
        setCursorVerticalHorizontalPosition(38 + menuOptions[previousSelection].row * 2, menuOptions[previousSelection].consoleColumn);
        printf("  %s", menuOptions[previousSelection].text);
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || !canAffordSelection || !unitHasValidCells);
    if (currentSelection == MENU_BACK) {
        for (int i = 0; i < 3; ++i) {
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
    for (int i = 0; i < 3; ++i) {
        List_clear(&unitValidCells[i]);
    }
    if (selectedCell == NULL) return openSpawnUnitMenu(gameDataEx, currentSelection); // If it's NULL, the user must have pressed ESC.
    createEntity(selectedCell, gameDataEx->currentPlayer, selectedUnit);
    setCursorAtCellCoord(selectedCellCoord);
    printGameCell(gameDataEx->gameData, selectedCell, false);
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

void drawEntityStatus(const GameData* gameData, const Int16Vector2 cellCoord, const uint16_t damage, const uint16_t column) {
    const GameBoardCell *cell = &gameData->board[cellCoord.y][cellCoord.x];
    setCursorVerticalHorizontalPosition(40, column);
    clearFromCursorForward(30);
    printf("%s", getEntityName(cell->entityType, gameData->players[cell->owner].isMordor));
    setCursorVerticalHorizontalPosition(40, column + (cellCoord.y > 9 ? 26 : 27));
    printf("%c-%d", 'A' + cellCoord.x, cellCoord.y);
    const uint16_t maxHealth = getEntityInfo(cell->entityType).maxHealth;
    const double remainingHPFactor = (double)cell->health / maxHealth;
    uint16_t remainingHpBarWidth = (uint16_t)(round(remainingHPFactor * 30));
    uint16_t damageBarWidth = 0;
    if (damage) {
        damageBarWidth = remainingHpBarWidth;
        damageBarWidth -= (uint16_t)(round(fmax(cell->health - damage, 0) / maxHealth * 30));
        remainingHpBarWidth -= damageBarWidth;
    }
    const uint16_t lostHpBarWidth = max(30 - remainingHpBarWidth - damageBarWidth, 0);
    if (remainingHPFactor <= 1.0/3) {
        setForegroundColor(RED);
    } else if (remainingHPFactor <= 2.0/3) {
        setForegroundColor(YELLOW);
    } else {
        setForegroundColor(GREEN);
    }
    setCursorVerticalHorizontalPosition(42, column);
    clearFromCursorForward(30);
    printf("HP %d/%d", cell->health, maxHealth);
    setCursorVerticalHorizontalPosition(41, column);
    clearFromCursorForward(30);
    for (uint16_t i = 0; i < remainingHpBarWidth; ++i) {
        printf("\u2584");
    }
    if (damageBarWidth) {
        setForegroundColor(RED);
        enableBlinking();
        for (uint16_t i = 0; i < damageBarWidth; ++i) {
            printf("\u2584");
        }
        disableBlinking();
    }
    if (lostHpBarWidth) {
        setForegroundColor(DARK_GRAY);
        for (uint16_t i = 0; i < lostHpBarWidth; ++i) {
            printf("\u2584");
        }
    }
    if (damage) {
        setCursorVerticalHorizontalPosition(42, (uint16_t)(column + 26 - getNumDigits(damage)));
        setForegroundColor(RED);
        enableBlinking();
        printf("-%d HP", damage);
        disableBlinking();
    }
    resetForegroundColor();
}

Int16Vector2* makeEntitySelection(const GameDataExtended* gameDataEx, const bool isUnitSelection) {
    const List list = isUnitSelection ? gameDataEx->currentPlayerUnitsList : gameDataEx->currentPlayerBuildingsList;
    Node *node = getNextInList(list, NULL);
    if (node == NULL) return NULL;
    printEntityList(gameDataEx, isUnitSelection ? 2 : 1);
    drawActionsMenu(isUnitSelection ? "Select Unit..." : "Select Building...", MENU_FOOTER_CANCEL, true);
    setCursorVerticalHorizontalPosition(40, 50);
    printf("Use the [↑]/[↓] keys to navigate your %s.", isUnitSelection ? "units" : "buildings");
    setCursorVerticalHorizontalPosition(42, 50);
    printf("Press [SPACE] or [ENTER] to confirm the selection.");
    Int16Vector2 *selectedCellCoord;
    const Node *previousNode = NULL;
    int ch;
    do {
        selectedCellCoord = node->data;
        const GameBoardCell *selectedCell = &gameDataEx->gameData->board[selectedCellCoord->y][selectedCellCoord->x];
        if (node != previousNode) {
            setCursorAtCellCoord(*selectedCellCoord);
            enableBlinking();
            printGameCell(gameDataEx->gameData, selectedCell, true);
            resetBackgroundColor();
            disableBlinking();
            drawEntityStatus(gameDataEx->gameData, *selectedCellCoord, 0, 12);
            previousNode = node;
        }
        bool arrowKeyPressed = false;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        switch (ch) {
            case KEY_W: case KEY_UP:   node = getPreviousInList(list, node); arrowKeyPressed = true; break;
            case KEY_S: case KEY_DOWN: node = getNextInList(list, node);     arrowKeyPressed = true; break;
            case KEY_ENTER: case KEY_SPACE: case KEY_ESC: break;
            default: continue;
        }
        if (arrowKeyPressed && node == previousNode) continue;
        // Here `selectedCell` and `selectedCellCoord` still refer to the previously selected cell.
        setCursorAtCellCoord(*selectedCellCoord);
        printGameCell(gameDataEx->gameData, selectedCell, false);
        resetBackgroundColor();
        if (ch == KEY_ESC) return NULL;
    } while (ch != KEY_ENTER && ch != KEY_SPACE);
    return selectedCellCoord;
}

int32_t openSelectBuildingMenu(const GameDataExtended* gameDataEx) {
    const Int16Vector2 *selectedCellCoord = makeEntitySelection(gameDataEx, false);
    if (selectedCellCoord == NULL) return MENU_BACK;
    GameBoardCell *cell = &gameDataEx->gameData->board[selectedCellCoord->y][selectedCellCoord->x];
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
                cell->health = 0;
                processPlayerLoss(gameDataEx->gameData, cell->owner);
                break;
            }
            cell->entityType = EMPTY_CELL;
            cell->health = 0;
            cell->owner = NO_OWNER;
            setCursorAtCellCoord(*selectedCellCoord);
            printGameCell(gameDataEx->gameData, cell, false);
            resetBackgroundColor();
            break;
    }
    return selection;
}

uint8_t enterMoveUnitMode(const GameDataExtended* gameDataEx, Int16Vector2* unitCoord, const List validCellsToMoveTo) {
    const GameBoardCell *unitCurrentCell = &gameDataEx->gameData->board[unitCoord->y][unitCoord->x];
    drawActionsMenu("Moving Unit...", MENU_FOOTER_CANCEL, false);
    drawValidCells(gameDataEx->gameData, validCellsToMoveTo, true);
    Int16Vector2 selectedCoord = *unitCoord;
    bool isSelectedCellValid, canAffordSelection = true;
    int ch;
    do {
        const GameBoardCell* selectedCell = &gameDataEx->gameData->board[selectedCoord.y][selectedCoord.x];
        setCursorAtCellCoord(selectedCoord);
        enableBlinking();
        if (selectedCell == unitCurrentCell) {
            printGameCell(gameDataEx->gameData, selectedCell, false);
            resetBackgroundColor();
            isSelectedCellValid = false;
        } else if ((isSelectedCellValid = isValidCell(selectedCoord, validCellsToMoveTo))) {
            GameBoardCell tempCell = *unitCurrentCell;
            tempCell.terrainType = selectedCell->terrainType;
            printGameCell(gameDataEx->gameData, &tempCell, true);
            resetBackgroundColor();
        } else {
            setBackgroundColor(getTerrainBackgroundColor(selectedCell->terrainType));
            setForegroundColor(RED);
            printf("╳╳╳");
            resetBackgroundColor();
            resetForegroundColor();
        }
        disableBlinking();
        if (selectedCell == unitCurrentCell) {
            setCursorVerticalHorizontalPosition(41, 45);
            clearFromCursorForward(62);
            setCursorVerticalHorizontalPosition(40, 50);
            clearFromCursorForward(57);
            printf("Use the arrow keys or WASD to move the unit.");
            setCursorVerticalHorizontalPosition(42, 50);
            clearFromCursorForward(57);
            printf("Press [SPACE] or [ENTER] to confirm the location.");
        } else {
            setCursorVerticalHorizontalPosition(40, 50);
            clearFromCursorForward(57);
            setCursorVerticalHorizontalPosition(42, 50);
            clearFromCursorForward(57);
            setCursorVerticalHorizontalPosition(41, 45);
            clearFromCursorForward(62);
            printf("━━━━━━━❯");
            if (isSelectedCellValid) {
                setCursorVerticalHorizontalPosition(40, 56);
                printf("Move from %c-%d (%s) to %c-%d (%s)...",
                    'A' + unitCoord->x, unitCoord->y, getTerrainName(unitCurrentCell->terrainType),
                    'A' + selectedCoord.x, selectedCoord.y, getTerrainName(selectedCell->terrainType)
                );
                const uint16_t totalMovementCost = getMovementCost(gameDataEx->gameData, *unitCoord, selectedCoord);
                setCursorVerticalHorizontalPosition(42, 56);
                printf("Movement Cost: ");
                if ((canAffordSelection = totalMovementCost <= gameDataEx->currentPlayer->coins)) {
                    setForegroundColor(GREEN);
                    printf("₵%d", totalMovementCost);
                    resetForegroundColor();
                } else {
                    setForegroundColor(RED);
                    printf("₵%d (Insufficient Castar Coins)", totalMovementCost);
                    resetForegroundColor();
                }
            } else {
                setCursorVerticalHorizontalPosition(41, 56);
                setForegroundColor(RED);
                printf("Cannot move unit to the selected location.");
                resetForegroundColor();
            }
        }
        const Int16Vector2 previousCoord = selectedCoord;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        switch (ch) {
            case KEY_W: case KEY_UP:    --selectedCoord.y; break;
            case KEY_S: case KEY_DOWN:  ++selectedCoord.y; break;
            case KEY_D: case KEY_RIGHT: ++selectedCoord.x; break;
            case KEY_A: case KEY_LEFT:  --selectedCoord.x; break;
            case KEY_ENTER: case KEY_SPACE: case KEY_ESC:  break;
            default: continue;
        }
        selectedCoord.x = (int16_t)((selectedCoord.x + 26) % 26);
        selectedCoord.y = (int16_t)((selectedCoord.y + 17) % 17);
        setCursorAtCellCoord(previousCoord);
        if (selectedCell == unitCurrentCell) {
            // Here `selectedCell` still refers to the previously selected cell.
            const GameBoardCell tempCell = {unitCurrentCell->terrainType, EMPTY_CELL, NO_OWNER, 0, false};
            printGameCell(gameDataEx->gameData, &tempCell, false);
        } else {
            printGameCell(gameDataEx->gameData, selectedCell, isSelectedCellValid);
        }
        resetBackgroundColor();
        if (ch == KEY_ESC) {
            setCursorAtCellCoord(*unitCoord);
            printGameCell(gameDataEx->gameData, unitCurrentCell, false);
            resetBackgroundColor();
            drawValidCells(gameDataEx->gameData, validCellsToMoveTo, false);
            return 0;
        }
    } while ((ch != KEY_ENTER && ch != KEY_SPACE) || !isSelectedCellValid || !canAffordSelection);
    drawValidCells(gameDataEx->gameData, validCellsToMoveTo, false);
    if (!moveUnit(gameDataEx->gameData, *unitCoord, selectedCoord)) {
        setCursorAtCellCoord(*unitCoord);
        printGameCell(gameDataEx->gameData, unitCurrentCell, false);
        resetBackgroundColor();
        return 1;
    }
    *unitCoord = selectedCoord;
    return 2;
}

bool enterAttackMode(const GameDataExtended* gameDataEx, const Int16Vector2* unitCoord, const List enemiesInRange) {
    drawActionsMenu("Attack...", MENU_FOOTER_CANCEL, false);
    setCursorVerticalHorizontalPosition(44, 58);
    printf("Press [↑]/[↓] to cycle through enemies in range");
    setCursorVerticalHorizontalPosition(45, 69);
    printf(" Press [SPACE] or [ENTER] to Confirm ");
    Node *node = getNextInList(enemiesInRange, NULL);
    const Node *previousNode = NULL;
    Int16Vector2 *selectedCellCoord;
    int ch;
    do {
        selectedCellCoord = node->data;
        const GameBoardCell* selectedCell = &gameDataEx->gameData->board[selectedCellCoord->y][selectedCellCoord->x];
        if (node != previousNode) {
            setCursorAtCellCoord(*selectedCellCoord);
            enableBlinking();
            printGameCell(gameDataEx->gameData, selectedCell, true);
            resetBackgroundColor();
            disableBlinking();
            setCursorVerticalHorizontalPosition(40, 50);
            clearFromCursorForward(57);
            setCursorVerticalHorizontalPosition(42, 50);
            clearFromCursorForward(57);
            setCursorVerticalHorizontalPosition(41, 50);
            clearFromCursorForward(57);
            printf("━ ATTACK ━❯");
            const uint16_t damage = getAttackPower(gameDataEx->gameData, *unitCoord, *selectedCellCoord);
            drawEntityStatus(gameDataEx->gameData, *selectedCellCoord, damage, 69);
            setCursorVerticalHorizontalPosition(41, 69);
            previousNode = node;
        }
        bool arrowKeyPressed = false;
        ch = _getch();
        if (ch == 0 || ch == 224) ch = _getch();
        switch (ch) {
            case KEY_W: case KEY_UP:   node = getPreviousInList(enemiesInRange, node); arrowKeyPressed = true; break;
            case KEY_S: case KEY_DOWN: node = getNextInList(enemiesInRange, node);     arrowKeyPressed = true; break;
            case KEY_ENTER: case KEY_SPACE: case KEY_ESC: break;
            default: continue;
        }
        setCursorVerticalHorizontalPosition(44, 58);
        clearFromCursorForward(49);
        if (arrowKeyPressed && node == previousNode) continue;
        // Here `selectedCell` and `selectedCellCoord` still refer to the previously selected cell.
        setCursorAtCellCoord(*selectedCellCoord);
        printGameCell(gameDataEx->gameData, selectedCell, false);
        resetBackgroundColor();
        if (ch == KEY_ESC) return false;
    } while (ch != KEY_ENTER && ch != KEY_SPACE);
    performAttack(gameDataEx->gameData, *unitCoord, *selectedCellCoord);
    return true;
}

void openSelectUnitMenu(GameDataExtended* gameDataEx, Int16Vector2* selectedCellCoord, int32_t currentSelection) {
    if (selectedCellCoord == NULL) {
        selectedCellCoord = makeEntitySelection(gameDataEx, true);
        if (selectedCellCoord == NULL) return;
    }
    drawActionsMenu("Unit Actions", MENU_FOOTER_GO_BACK, false);
    setCursorVerticalHorizontalPosition(40, 43);
    clearFromCursorForward(64);
    setCursorVerticalHorizontalPosition(41, 43);
    clearFromCursorForward(64);
    setCursorVerticalHorizontalPosition(42, 43);
    clearFromCursorForward(64);
    List validCellsToMoveTo = getValidCellsToMoveUnit(gameDataEx, *selectedCellCoord);
    List enemiesInRange = getEnemyEntitiesInRange(gameDataEx, *selectedCellCoord);
    ActionMenuOption menuActions[] = {
        (gameDataEx->currentPlayer->coins <= 0
        ? (ActionMenuOption){"Move (Not Enough Castar Coins)", 0, true, 1, 50}
        : validCellsToMoveTo.length > 0
        ? (ActionMenuOption){"Move", 0, false, 1, 50}
        : (ActionMenuOption){"Move (Path Obstructed)", 0, true, 1, 50}),
        (gameDataEx->gameSettings->ENTITIES_MAY_ONLY_ATTACK_ONCE_PER_ROUND && gameDataEx->gameData->board[selectedCellCoord->y][selectedCellCoord->x].hasAttackedThisRound
        ? (ActionMenuOption){"Attack (Already Attacked this Round)", 0, true, 2, 50}
        : enemiesInRange.length > 0
        ? (ActionMenuOption){"Attack", 0, false, 2, 50}
        : (ActionMenuOption){"Attack (Not in Range)", 0, true, 2, 50})
    };
    const int32_t selection = makeActionMenu(gameDataEx, menuActions, 2, &currentSelection);
    if (selection == MENU_BACK) {
        List_clear(&validCellsToMoveTo);
        List_clear(&enemiesInRange);
        return;
    }
    setCursorVerticalHorizontalPosition(40, 50);
    clearFromCursorForward(57);
    setCursorVerticalHorizontalPosition(42, 50);
    clearFromCursorForward(57);
    if (selection == 0) {
        do {
            const uint8_t moveResult = enterMoveUnitMode(gameDataEx, selectedCellCoord, validCellsToMoveTo);
            if (moveResult != 2) break;
            List_clear(&validCellsToMoveTo);
            validCellsToMoveTo = getValidCellsToMoveUnit(gameDataEx, *selectedCellCoord);
            drawEntityStatus(gameDataEx->gameData, *selectedCellCoord, 0, 12);
            printEntityList(gameDataEx, 2);
            printTurnInfoBox(gameDataEx);
        } while (validCellsToMoveTo.length && gameDataEx->currentPlayer->coins > 0);
    } else {
        enterAttackMode(gameDataEx, selectedCellCoord, enemiesInRange);
    }
    List_clear(&validCellsToMoveTo);
    List_clear(&enemiesInRange);
    if (!menuActions[0].disabled || !menuActions[1].disabled) {
        openSelectUnitMenu(gameDataEx, selectedCellCoord, currentSelection);
    }
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

void Game(GameData* gameData, const uint8_t saveSlot) {
    const time_t startTime = time(NULL);
    GameDataExtended gameDataEx = {
        gameData,
        NULL,
        {0, 0},
        createInt16Vector2List(),
        createInt16Vector2List(),
        getGameSettings()
    };
    int32_t currentSelection = 0;
    gameDataEx.currentPlayer = &gameDataEx.gameData->players[gameDataEx.gameData->currentPlayerTurn];
    gameDataEx.currentPlayerBaseCoord = getPlayerBaseCoordinate(gameDataEx.gameData, gameDataEx.gameData->currentPlayerTurn);
    clearConsole();
    printGameBoard(gameDataEx.gameData);
    bool shouldRefreshEntityLists = true;
    while (!gameDataEx.gameData->isGameOver) {
        if (shouldRefreshEntityLists) {
            refreshCurrentPlayerEntityLists(&gameDataEx);
            shouldRefreshEntityLists = false;
        }
        printEntityList(&gameDataEx, 0);
        printTurnInfoBox(&gameDataEx);
        while (true) {
            if (enforceConsoleResize(NULL, NULL, 135, 46)) {
                printGameBoard(gameDataEx.gameData);
                printEntityList(&gameDataEx, 0);
                printTurnInfoBox(&gameDataEx);
            }
            const int32_t action = openMainActionsMenu(&gameDataEx, &currentSelection);
            if (action == 0) {
                shouldRefreshEntityLists = openCreateBuildingMenu(&gameDataEx, 0) != MENU_BACK;
                break;
            }
            if (action == 1) {
                shouldRefreshEntityLists = openSpawnUnitMenu(&gameDataEx, 0) != MENU_BACK;
                break;
            }
            if (action == 2) {
                shouldRefreshEntityLists = openSelectBuildingMenu(&gameDataEx) != MENU_BACK;
                break;
            }
            if (action == 3) {
                openSelectUnitMenu(&gameDataEx, NULL, 0);
                break;
            }
            if (action == 4) {
                advanceTurn(&gameDataEx);
                currentSelection = 0;
                break;
            }
            // If we get here then the user pressed ESC or selected "Exit Game"
            const int32_t exitResult = openExitGameMenu();
            switch (exitResult) {
                case 0: case 1:
                    gameDataEx.gameData->elapsedTimeSeconds += time(NULL) - startTime;
                    saveGame(gameDataEx.gameData, saveSlot);
                    break;
            }
            switch (exitResult) {
                case 0: case 2: exit(0);
                case 1: case 3:
                    List_clear(&gameDataEx.currentPlayerBuildingsList);
                    List_clear(&gameDataEx.currentPlayerUnitsList);
                    return; // Return to Main Menu.
            }
            // If we get here then the user pressed ESC on the "Exit Game" menu, so we return to the Main Actions menu.
        }
        if (gameDataEx.gameData->isGameOver) break;
        if (!gameDataEx.gameSettings->ENTITIES_MAY_ONLY_ATTACK_ONCE_PER_ROUND && gameDataEx.currentPlayer->coins <= 0) {
            printTurnInfoBox(&gameDataEx);
            drawActionsMenu(NULL, NULL, true);
            setCursorVerticalHorizontalPosition(40, 16);
            printf("You have run out of Castar Coins and cannot perform any more actions this turn.");
            setCursorVerticalHorizontalPosition(42, 54);
            enableBlinking();
            printf("\x1B[4mOK\x1B[24m");
            disableBlinking();
            int ch;
            do {
                ch = _getch();
            } while (ch != KEY_ENTER && ch != KEY_SPACE);
            advanceTurn(&gameDataEx);
            currentSelection = 0;
        }
    }
    printEntityList(&gameDataEx, 0);
    List_clear(&gameDataEx.currentPlayerBuildingsList);
    List_clear(&gameDataEx.currentPlayerUnitsList);
    // If we get here then the game is over.
    const Player *winner = NULL;
    for (uint8_t i = 0; i < gameDataEx.gameData->nPlayers; ++i) {
        if (gameDataEx.gameData->players[i].isAlive) {
            winner = &gameDataEx.gameData->players[i];
            break;
        }
    }
    assert(winner != NULL);
    drawActionsMenu(NULL, "Press [SPACE] or [ENTER] to return to the Main Menu", true);
    setCursorVerticalHorizontalPosition(40, 51);
    printf("Game Over");
    setCursorVerticalHorizontalPosition(42, (uint16_t)((90 - strlen(winner->name)) / 2 + 4));
    printf("The winner is %s!", winner->name);
    int ch;
    do {
        ch = _getch();
    } while (ch != KEY_ENTER && ch != KEY_SPACE);
}

void createBoardFromMapFile(char* const mapFileName, GameData* gameData) {
    FILE *fp = fopen(mapFileName, "r");
    if (fp == NULL) return;
    uint8_t player = 0;
    for (uint16_t y = 0; y < 17; ++y) {
        for (uint16_t x = 0; x < 26; ++x) {
            const int ch = fgetc(fp);
            if (ch == EOF) {
                y = 17;
                break;
            }
            switch (ch) {
                default:
                case 'P': gameData->board[y][x].terrainType = PLAIN;    break;
                case 'F': gameData->board[y][x].terrainType = FOREST;   break;
                case 'M': gameData->board[y][x].terrainType = MOUNTAIN; break;
                case 'R': gameData->board[y][x].terrainType = RIVER;    break;
                case 'W': gameData->board[y][x].terrainType = WATER;    break;
                case 'B': gameData->board[y][x].terrainType = BRIDGE;   break;
                case 'S': gameData->board[y][x].terrainType = SNOW;     break;
                case 'L': gameData->board[y][x].terrainType = LAVA;     break;
                case 'T': gameData->board[y][x].terrainType = BASALT;   break;
                case '#': createEntity(&gameData->board[y][--x], &gameData->players[player++], BASE); break;
            }
        }
        // Skip the line feed (\n) at the end of each row
        (void)fgetc(fp);
    }
    (void)fclose(fp);
    free(mapFileName);
}

void assignRandomPlayerBases(GameData* gameData) {
    srand((uint64_t)time(NULL) % UINT32_MAX);
    for (uint8_t pId = 0; pId < gameData->nPlayers; ++pId) {
        uint16_t y, x, n = 0;
        do {
            y = rand() % 15 + 1;
            x = gameData->players[pId].isMordor ? 24 - rand() % 2 : rand() % 2 + 1;
            ++n;
        } while (gameData->board[y][x].entityType != EMPTY_CELL && n < 100);
        createEntity(&gameData->board[y][x], &gameData->players[pId], BASE);
    }
}

void startNewSinglePlayerGame(const uint8_t saveSlot, char* const mapFile, const char* const playerName, const bool player1IsMordor) {
    GameData gameData = {
        time(NULL),
        time(NULL),
        0,
        0,
        0,
        false,
        {{{PLAIN, EMPTY_CELL, NO_OWNER, 0, false}}},
        2,
        {
            {0, "Player 1", 100, player1IsMordor, true, false },
            {1, "CPU", 100, !player1IsMordor, true, true }
        }
    };
    if (mapFile != NULL) createBoardFromMapFile(mapFile, &gameData);
    else assignRandomPlayerBases(&gameData);
    strcpy(gameData.players[0].name, playerName);
    Game(&gameData, saveSlot);
}

void startNewMultiplayerGame(const uint8_t saveSlot, char* const mapFile, const char* const player1Name, const char* const player2Name, const bool player1IsMordor) {
    GameData gameData = {
        time(NULL),
        time(NULL),
        0,
        0,
        0,
        false,
        {{{PLAIN, EMPTY_CELL, NO_OWNER, 0, false}}},
        2,
        {
            {0, "Player 1", 100, player1IsMordor, true, false },
            {1, "Player 2", 100, !player1IsMordor, true, false }
        }
    };
    if (mapFile != NULL) createBoardFromMapFile(mapFile, &gameData);
    else assignRandomPlayerBases(&gameData);
    strcpy(gameData.players[0].name, player1Name);
    strcpy(gameData.players[1].name, player2Name);
    Game(&gameData, saveSlot);
}

void resumeSaveGame(const uint8_t saveSlot) {
    GameData *gameData;
    if (!loadGame(&gameData, saveSlot)) return;
    Game(gameData, saveSlot);
    free(gameData);
}
