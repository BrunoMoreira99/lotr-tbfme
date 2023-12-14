#pragma once
#include <stdint.h>

#define NO_OWNER _UI8_MAX // 255 / 0xFF

typedef enum {
    PLAIN,
    FOREST,
    MOUNTAIN,
    RIVER,
    WATER,
    BRIDGE,
    SNOW,
    LAVA
} TerrainType;

typedef enum {
    EMPTY_CELL,
    BASE,
    MINES,
    BARRACKS,
    STABLES,
    ARMOURY,
    INFANTRY,
    CAVALRY,
    ARTILLERY
} EntityType;

typedef struct {
    uint8_t id;
    char name[20];
    int32_t coins;
    bool isMordor;
    bool isCPU;
} Player;

typedef struct {
    TerrainType terrainType;
    EntityType entityType;
    uint8_t owner;
    int16_t health;
} GameBoardCell;

typedef struct {
    uint64_t timestamp;
    uint32_t currentRound;
    Player players[2];
    uint8_t currentPlayerTurn;
    bool isGameOver;
    GameBoardCell board[17][26];
} GameData;

typedef struct {
    uint16_t MINE_INCOME;
    // Building Health
    uint16_t BASE_HEALTH;
    uint16_t MINES_HEALTH;
    uint16_t BARRACKS_HEALTH;
    uint16_t STABLES_HEALTH;
    uint16_t ARMOURY_HEALTH;
    // Unit Health
    uint16_t INFANTRY_HEALTH;
    uint16_t CAVALRY_HEALTH;
    uint16_t ARTILLERY_HEALTH;
    // Building Cost
    uint16_t BASE_COST;
    uint16_t MINES_COST;
    uint16_t BARRACKS_COST;
    uint16_t STABLES_COST;
    uint16_t ARMOURY_COST;
    // Unit Spawn Cost
    uint16_t INFANTRY_SPAWN_COST;
    uint16_t CAVALRY_SPAWN_COST;
    uint16_t ARTILLERY_SPAWN_COST;
    // Unit Movement Cost
    uint16_t INFANTRY_MOVEMENT_COST;
    uint16_t CAVALRY_MOVEMENT_COST;
    uint16_t ARTILLERY_MOVEMENT_COST;
    // Unit Attack Power
    uint16_t INFANTRY_ATTACK_POWER;
    uint16_t CAVALRY_ATTACK_POWER;
    uint16_t ARTILLERY_ATTACK_POWER;
} GameSettings;

extern const GameSettings GameSettings_Default;
extern GameSettings *GameSettings_Current;

GameSettings* getGameSettings(void);

void printGameBoard(const GameData* gameData);
void startNewSinglePlayerGame(char* mapFile, const char* playerName, const bool player1IsMordor);
void startNewMultiplayerGame(char* mapFile, const char* player1Name, const char* player2Name, const bool player1IsMordor);
